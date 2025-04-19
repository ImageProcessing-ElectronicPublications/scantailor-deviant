/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DewarpingImageTransform.h"
#include "RasterDewarper.h"
#include "foundation/RoundingHasher.h"
#include "STEX_ToVec.h"
#include "ToLineProjector.h"
#include "math/LineBoundedByPolygon.h"
#include "imageproc/AffineTransformedImage.h"
#include <Eigen/Core>
#include <Eigen/LU>
#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <map>

using namespace Eigen;
using namespace imageproc;

namespace dewarping
{

class DewarpingImageTransform::ConstrainedCropAreaBuilder
{
public:
    ConstrainedCropAreaBuilder(QPolygonF const& orig_crop_area,
                               double min_density, double max_density, CylindricalSurfaceDewarper const& dewarper);

    /**
     * Sample crv_x values in a certain range with dynamically adjusted step size.
     * Each sample contributes two points to the resulting crop area.
     */
    void sampleCrvXRange(double from, double to, double forward_direction);

    /**
     * Return the crop area resulting from calls to sampleCrvXRange().
     */
    QPolygonF build() const;
private:
    /**
     * Determines the vertical boundaries of the generatrix and adds the corresponding
     * line segment to m_vertSegments. Returns an iterator to the newly added element in
     * m_vertSegments or m_vertSegments.end(), if the generatrix is outside of a feasible region.
     */
    std::map<double, QLineF>::iterator processGeneratrix(
        double crv_x, CylindricalSurfaceDewarper::Generatrix const& generatrix);

    /**
     * If lengths of two consecutive segments differ significantly, create
     * another segment in the middle. Proceed recursively from there.
     */
    void maybeAddExtraVerticalSegments(
        double segment1_crv_x, double segment1_len,
        double segment2_crv_x, double segment2_len);

    QPolygonF const& m_origCropArea;

    /**
     * Densities are expressed as the length in pixels in original image over
     * the corresponding length in crv_y units. For an explanation of crv_*
     * coordinates, see comments at the top of CylindricalSurfaceDewarper.cpp.
     */
    double const m_minDensity;
    double const m_maxDensity;

    CylindricalSurfaceDewarper const& m_dewarper;
    CylindricalSurfaceDewarper::State m_dewarpingState;

    /**
     * A collection of line segments in original image coordinates corresponding to
     * vertical line segments in dewarped image coordinates. Indexed and ordered by crv_x.
     * QLineF has the layout of (top_point, bottom_point) when viewed from dewarped point
     * of view. For an explanation of crv_* coordinates, see comments at the top of
     * CylindricalSurfaceDewarper.cpp.
     */
    std::map<double, QLineF> m_vertSegments;
};


DewarpingImageTransform::DewarpingImageTransform(
    QSize const& orig_size,
    QPolygonF const& orig_crop_area,
    std::vector<QPointF> const& top_curve,
    std::vector<QPointF> const& bottom_curve,
    FovParams const& fov_params,
    FrameParams const& frame_params,
    BendParams const& bend_params,
    SizeParams const& size_params)
    :	m_origSize(orig_size)
    ,	m_topPolyline(top_curve)
    ,	m_bottomPolyline(bottom_curve)
    ,   m_sizeParams(size_params)
    ,	m_dewarper(top_curve, bottom_curve,
                   fov_params, frame_params, bend_params)
    ,	m_intrinsicScaleX(1.0)
    ,	m_intrinsicScaleY(1.0)
    ,	m_userScaleX(1.0)
    ,	m_userScaleY(1.0)
{
    // These two lines don't depend on each other and therefore can go in any order.
    m_origCropArea = constrainCropArea(orig_crop_area);
    setupIntrinsicScale();
}

DewarpingImageTransform::~DewarpingImageTransform()
{
}

std::unique_ptr<AbstractImageTransform>
DewarpingImageTransform::clone() const
{
    return std::unique_ptr<AbstractImageTransform>(
               new DewarpingImageTransform(*this)
           );
}

QPolygonF
DewarpingImageTransform::transformedCropArea() const
{
    CylindricalSurfaceDewarper::State state;
    QPolygonF poly(m_origCropArea);

    for (QPointF& pt : poly)
    {
        pt = postScale(m_dewarper.mapToDewarpedSpace(pt, state));
    }

    return poly;
}

ImageSize
DewarpingImageTransform::imageSize() const
{
    return m_dewarper.imageSize(
        m_topPolyline,
        m_bottomPolyline,
        m_sizeParams
    );
}

QTransform
DewarpingImageTransform::scale(qreal xscale, qreal yscale)
{
    m_userScaleX *= xscale;
    m_userScaleY *= yscale;

    QTransform scaling_transform;
    scaling_transform.scale(xscale, yscale);
    return scaling_transform;
}

DewarpingImageTransform
DewarpingImageTransform::scaled(qreal xscale, qreal yscale) const
{
    DewarpingImageTransform transform(*this);
    transform.scale(xscale, yscale);
    return transform;
}

QImage
DewarpingImageTransform::materialize(QImage const& image,
                                     QRect const& target_rect, QColor const& outside_color) const
{
    assert(!image.isNull());
    assert(!target_rect.isEmpty());

    QRectF model_domain(0, 0, m_intrinsicScaleX * m_userScaleX, m_intrinsicScaleY * m_userScaleY);
    model_domain.translate(-target_rect.topLeft());

    return RasterDewarper::dewarp(
               image, target_rect.size(), m_dewarper, model_domain, outside_color
           );
}

std::function<QPointF(QPointF const&)>
DewarpingImageTransform::forwardMapper() const
{
    auto dewarper(std::make_shared<CylindricalSurfaceDewarper>(m_dewarper));
    QTransform post_transform;
    post_transform.scale(m_intrinsicScaleX * m_userScaleX, m_intrinsicScaleY * m_userScaleY);

    return [=](QPointF const& pt)
    {
        return post_transform.map(dewarper->mapToDewarpedSpace(pt));
    };
}

std::function<QPointF(QPointF const&)>
DewarpingImageTransform::backwardMapper() const
{
    auto dewarper(std::make_shared<CylindricalSurfaceDewarper>(m_dewarper));
    QTransform pre_transform;
    qreal const xscale = m_intrinsicScaleX * m_userScaleX;
    qreal const yscale = m_intrinsicScaleY * m_userScaleY;
    pre_transform.scale(1.0 / xscale, 1.0 / yscale);

    return [=](QPointF const& pt)
    {
        return dewarper->mapToWarpedSpace(pre_transform.map(pt));
    };
}

QPointF
DewarpingImageTransform::postScale(QPointF const& pt) const
{
    qreal const xscale = m_intrinsicScaleX * m_userScaleX;
    qreal const yscale = m_intrinsicScaleY * m_userScaleY;
    return QPointF(pt.x() * xscale, pt.y() * yscale);
}

/**
 * Initializes m_intrinsicScaleX and m_intrinsicScaleY in such a way that pixel
 * density near the "closest to the camera" corner of dewarping quadrilateral matches
 * the corresponding pixel density in a transformed image. Since we don't know
 * the physical dimensions of original image, we express pixel density in
 * "pixels / ABSTRACT_PHYSICAL_UNIT" units, where ABSTRACT_PHYSICAL_UNIT is
 * a hardcoded small fraction of either the physical width or the physical
 * height of dewarping quadrilateral.
 * Unfortunately, the approach above results in huge dewarped images in the presence
 * of a strong perspective distortion. Therefore, we additionally scale m_intrinsicScaleX
 * and m_intrinsicScaleY in such a way that the area (in pixels) of the curved quadrilateral
 * equals the area of the corresponding dewarped rectangle.
 */
void
DewarpingImageTransform::setupIntrinsicScale()
{
    ImageSize const image_size =
        m_dewarper.imageSize(m_topPolyline, m_bottomPolyline, m_sizeParams);

    m_intrinsicScaleX = image_size.width;
    m_intrinsicScaleY = image_size.height;
}

QPolygonF
DewarpingImageTransform::constrainCropArea(QPolygonF const& orig_crop_area) const
{
    auto const minmax_densities = calcMinMaxDensities();
    double const min_density = minmax_densities.first;
    double const max_density = minmax_densities.second;

    ConstrainedCropAreaBuilder builder(orig_crop_area, min_density, max_density, m_dewarper);

    builder.sampleCrvXRange(0.0 + 0.3, 0.0 - 0.6, -1.0);
    builder.sampleCrvXRange(1.0 - 0.3, 1.0 + 0.6, 1.0);

    return builder.build();
}

std::pair<double, double>
DewarpingImageTransform::calcMinMaxDensities() const
{
    CylindricalSurfaceDewarper::State state;
    CylindricalSurfaceDewarper::Generatrix const left_bound =
        m_dewarper.mapGeneratrix(0.0, state);
    CylindricalSurfaceDewarper::Generatrix const right_bound =
        m_dewarper.mapGeneratrix(1.0, state);

    // Calculate densities of pixels in original (warped) image at corners
    // of the the curved quadrilateral. We assume those are positive.
    double const left_bound_len = left_bound.imgLine.length();
    double const right_bound_len = right_bound.imgLine.length();
    double const corner_densities[] =
    {
        left_bound.pln2img.derivativeAt(0.0) * left_bound_len,
        left_bound.pln2img.derivativeAt(1.0) * left_bound_len,
        right_bound.pln2img.derivativeAt(0.0) * right_bound_len,
        right_bound.pln2img.derivativeAt(1.0) * right_bound_len
    };

    // We want to constrain the crop area in such a way that warped
    // pixel density in that area doesn't go out of a certain range.
    auto const minmax_densities = std::minmax_element(
                                      std::begin(corner_densities), std::end(corner_densities)
                                  );
    return std::make_pair(0.6 * *minmax_densities.first, 1.4 * *minmax_densities.second);
}


/*============================= ConstrainedCropAreaBuilder ==================================*/

DewarpingImageTransform::ConstrainedCropAreaBuilder::ConstrainedCropAreaBuilder(
    QPolygonF const& orig_crop_area, double min_density, double max_density,
    CylindricalSurfaceDewarper const& dewarper)
    : m_origCropArea(orig_crop_area)
    , m_minDensity(min_density)
    , m_maxDensity(max_density)
    , m_dewarper(dewarper)
{
}

void
DewarpingImageTransform::ConstrainedCropAreaBuilder::sampleCrvXRange(
    double const from, double const to, double const forward_direction)
{
    double const backwards_direction = -forward_direction;
    double direction = forward_direction;
    double step_size = 0.1;
    double const min_step_size = step_size / 8;

    struct LastSegment
    {
        double crv_x;
        double length;
    };

    boost::optional<LastSegment> last_segment;

    for (double crv_x = from;
            (crv_x - to) * (from - to) > -std::numeric_limits<double>::epsilon()
            && step_size > min_step_size - std::numeric_limits<double>::epsilon();
            crv_x += step_size * direction)
    {

        auto segment_it = processGeneratrix(
                              crv_x, m_dewarper.mapGeneratrix(crv_x, m_dewarpingState)
                          );
        if (segment_it == m_vertSegments.end())
        {
            step_size *= 0.5;
            direction = backwards_direction;
        }
        else
        {
            double const segment_len = segment_it->second.length();
            if (last_segment)
            {
                maybeAddExtraVerticalSegments(
                    last_segment->crv_x, last_segment->length, segment_it->first, segment_len
                );
            }

            last_segment = LastSegment{segment_it->first, segment_len};
            direction = forward_direction;
        }
    }
}

QPolygonF
DewarpingImageTransform::ConstrainedCropAreaBuilder::build() const
{
    QPolygonF new_crop_area(m_vertSegments.size() * 2);

    int i1 = 0;
    int i2 = new_crop_area.size() - 1;

    for (auto const& kv : m_vertSegments)
    {
        new_crop_area[i1] = kv.second.p1();
        new_crop_area[i2] = kv.second.p2();
        ++i1;
        --i2;
    }

    return new_crop_area;
}

std::map<double, QLineF>::iterator
DewarpingImageTransform::ConstrainedCropAreaBuilder::processGeneratrix(
    double const crv_x, CylindricalSurfaceDewarper::Generatrix const& generatrix)
{
    // A pair of lower and upper bounds for y coordinate in a unit square
    // corresponding to the curved quadrilateral.
    std::pair<boost::optional<double>, boost::optional<double>> valid_range;

    // Called for points where pixel density reaches the lower or upper threshold.
    auto const processCriticalPoint = [&generatrix, &valid_range]
                                      (double crv_y, bool upper_threshold)
    {

        if (!generatrix.pln2img.mirrorSide(crv_y))
        {
            double const second_deriv = generatrix.pln2img.secondDerivativeAt(crv_y);
            if (std::signbit(second_deriv) == upper_threshold)
            {
                assert(!valid_range.first);
                valid_range.first = crv_y;
            }
            else
            {
                assert(!valid_range.second);
                valid_range.second = crv_y;
            }

        }
    };

    double const recip_len = 1.0 / generatrix.imgLine.length();

    generatrix.pln2img.solveForDeriv(
        m_minDensity * recip_len,
        [processCriticalPoint](double crv_y)
    {
        processCriticalPoint(crv_y, /*upper_threshold=*/false);
    }
    );

    generatrix.pln2img.solveForDeriv(
        m_maxDensity * recip_len,
        [processCriticalPoint](double crv_y)
    {
        processCriticalPoint(crv_y, /*upper_threshold=*/true);
    }
    );

    QLineF bounded_line(generatrix.imgLine);
    if (!lineBoundedByPolygon(bounded_line, m_origCropArea))
    {
        return m_vertSegments.end();
    }

    ToLineProjector const projector(generatrix.imgLine);
    double min_proj = projector.projectionScalar(bounded_line.p1());
    double max_proj = projector.projectionScalar(bounded_line.p2());

    if (valid_range.first)
    {
        min_proj = std::max<double>(min_proj, generatrix.pln2img(*valid_range.first));
    }

    if (valid_range.second)
    {
        max_proj = std::min<double>(max_proj, generatrix.pln2img(*valid_range.second));
    }

    if (min_proj >= max_proj)
    {
        return m_vertSegments.end();
    }

    QPointF const p1(generatrix.imgLine.pointAt(min_proj));
    QPointF const p2(generatrix.imgLine.pointAt(max_proj));

    return m_vertSegments.emplace(crv_x, QLineF(p1, p2)).first;
}

void
DewarpingImageTransform::ConstrainedCropAreaBuilder::maybeAddExtraVerticalSegments(
    double const segment1_crv_x, double const segment1_len,
    double const segment2_crv_x, double const segment2_len)
{
    auto const lengths_close_enough = [](double len1, double len2)
    {
        return std::max(len1, len2) - std::min(len1, len2) < 0.1 * (len1 + len2);
    };

    if (lengths_close_enough(segment1_len, segment2_len))
    {
        return;
    }

    double const delta_crv_x = segment1_crv_x - segment2_crv_x;
    if ((delta_crv_x > -1e-8) && (delta_crv_x < 1e-8))
    {
        return;
    }

    double const mid_crv_x = 0.5 * (segment1_crv_x + segment2_crv_x);
    auto const segment_it = processGeneratrix(
                                mid_crv_x, m_dewarper.mapGeneratrix(mid_crv_x, m_dewarpingState)
                            );
    if (segment_it == m_vertSegments.end())
    {
        return;
    }

    double const mid_len = segment_it->second.length();
    maybeAddExtraVerticalSegments(segment1_crv_x, segment1_len, mid_crv_x, mid_len);
    maybeAddExtraVerticalSegments(mid_crv_x, mid_len, segment2_crv_x, segment2_len);
}

} // namespace dewarping
