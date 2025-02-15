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

#include "CylindricalSurfaceDewarper.h"
#include "Directrix.h"
#include "ToLineProjector.h"
#include "NumericTraits.h"
#include "ToDouble.h"
#include "STEX_ToVec.h"
#include "STEX_ToPoint.h"
#include "foundation/MultipleTargetsSupport.h"
#include "dewarping/FovParams.h"
#include "dewarping/FrameParams.h"
#include "dewarping/BendParams.h"
#include <Eigen/Core>
#include <Eigen/QR>
#include <QLineF>
#include <QtGlobal>
#include <QDebug>
#include <boost/foreach.hpp>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <stdexcept>
#include <string>

/*
Naming conventions:
img: Coordinates in the warped image.
pln: Coordinates on a plane where the 4 corner points of the curved
     quadrilateral are supposed to lie. In our model we assume that
     all 4 lie on the same plane. The corner points are mapped to
     the following points on the plane:
     * Start point of curve1 [top curve]: (0, 0)
     * End point of curve1 [top curve]: (1, 0)
     * Start point of curve2 [bottom curve]: (0, 1)
     * End point of curve2 [bottom curve]: (1, 1)
     pln and img coordinates are linked by a 2D homography,
     namely m_pln2img and m_img2pln.
crv: Dewarped normalized coordinates. crv X coordinates are linked
     to pln X ccoordinates through m_arcLengthMapper while the Y
     coordinates are linked by a one dimensional homography that's
     different for each generatrix.
*/

using namespace Eigen;

namespace dewarping
{

CylindricalSurfaceDewarper::CylindricalSurfaceDewarper(
    std::vector<QPointF> const& img_directrix1,
    std::vector<QPointF> const& img_directrix2,
    double depth_perception,
    FovParams const& fov_params,
    FrameParams const& frame_params,
    BendParams const& bend_params)
    :   m_pln2img(calcPlnToImgHomography(img_directrix1, img_directrix2)),
      m_img2pln(m_pln2img.inv()),
      m_mdl2img(calcMdlToImgTransform(m_pln2img, fov_params, frame_params)),
      m_depthPerception(depth_perception),
      m_directrixArcLength(1.0),
      m_imgDirectrix1Intersector(img_directrix1),
      m_imgDirectrix2Intersector(img_directrix2)
{
    initArcLengthMapper(img_directrix1, img_directrix2);
}

CylindricalSurfaceDewarper::Generatrix
CylindricalSurfaceDewarper::mapGeneratrix(double crv_x, State& state) const
{
    ArcLengthMapper::XSample const sample = m_arcLengthMapper.arcLenToXSample(crv_x, state.m_arcLengthHint);

    Vector3d const mdl_top_pt(sample.x, 0.0, sample.fx);
    Vector3d const mdl_middle_pt(sample.x, 0.5, sample.fx);
    Vector3d const mdl_bottom_pt(sample.x, 1.0, sample.fx);

    QPointF const img_top_pt = toPoint(m_mdl2img(mdl_top_pt));
    QPointF const img_middle_pt = toPoint(m_mdl2img(mdl_middle_pt));
    QPointF const img_bottom_pt = toPoint(m_mdl2img(mdl_bottom_pt));

    QLineF const img_generatrix(img_top_pt, img_bottom_pt);

    QPointF const img_directrix1_pt(
        m_imgDirectrix1Intersector.intersect(img_generatrix, state.m_intersectionHint1)
    );
    QPointF const img_directrix2_pt(
        m_imgDirectrix2Intersector.intersect(img_generatrix, state.m_intersectionHint2)
    );

    ToLineProjector const projector(img_generatrix);

    double const img_middle_proj = projector.projectionScalar(img_middle_pt);

    boost::array<std::pair<double, double>, 3> img2pln_pairs = {
        std::make_pair(0.0, 0.0),
        std::make_pair(img_middle_proj, 0.5),
        std::make_pair(1.0, 1.0)
    };
    HomographicTransform<1, double> img2pln = threePoint1DHomography(img2pln_pairs);

    double const img_directrix1_proj = projector.projectionScalar(img_directrix1_pt);
    double const img_directrix2_proj = projector.projectionScalar(img_directrix2_pt);

    double const pln_directrix1_proj = toDouble(img2pln(toVec(img_directrix1_proj)));
    double const pln_directrix2_proj = toDouble(img2pln(toVec(img_directrix2_proj)));

    double const pln_middle_corrected_proj = 0.5 * (pln_directrix1_proj + pln_directrix2_proj);
    Vector3d const mdl_middle_corrected_pt(sample.x, pln_middle_corrected_proj, sample.fx);
    QPointF const img_middle_corrected_pt = toPoint(m_mdl2img(mdl_middle_corrected_pt));
    double const img_middle_corrected_proj = projector.projectionScalar(img_middle_corrected_pt);

    boost::array<std::pair<double, double>, 3> pln2img_pairs = {
        std::make_pair(0.0, img_directrix1_proj),
        std::make_pair(pln_middle_corrected_proj, img_middle_corrected_proj),
        std::make_pair(1.0, img_directrix2_proj)
    };
    HomographicTransform<1, double> pln2img = threePoint1DHomography(pln2img_pairs);

    return Generatrix(img_generatrix, pln2img);
}

QPointF
CylindricalSurfaceDewarper::mapToDewarpedSpace(QPointF const& img_pt) const
{
    State state;
    return mapToDewarpedSpace(img_pt, state);
}

QPointF
CylindricalSurfaceDewarper::mapToDewarpedSpace(QPointF const& img_pt, State& state) const
{
    double const pln_x = m_img2pln(toVec(img_pt))[0];
    ArcLengthMapper::ArcLenSample const sample = m_arcLengthMapper.xToArcLenSample(pln_x, state.m_arcLengthHint);

    Vector3d const mdl_top_pt(pln_x, 0.0, sample.fx);
    Vector3d const mdl_middle_pt(pln_x, 0.5, sample.fx);
    Vector3d const mdl_bottom_pt(pln_x, 1.0, sample.fx);

    QPointF const img_top_pt = toPoint(m_mdl2img(mdl_top_pt));
    QPointF const img_middle_pt = toPoint(m_mdl2img(mdl_middle_pt));
    QPointF const img_bottom_pt = toPoint(m_mdl2img(mdl_bottom_pt));

    QLineF const img_generatrix(img_top_pt, img_bottom_pt);

    QPointF const img_directrix1_pt(
        m_imgDirectrix1Intersector.intersect(img_generatrix, state.m_intersectionHint1)
    );
    QPointF const img_directrix2_pt(
        m_imgDirectrix2Intersector.intersect(img_generatrix, state.m_intersectionHint2)
    );

    ToLineProjector const projector(img_generatrix);

    double const img_middle_proj = projector.projectionScalar(img_middle_pt);

    boost::array<std::pair<double, double>, 3> img2pln_pairs = {
        std::make_pair(0.0, 0.0),
        std::make_pair(img_middle_proj, 0.5),
        std::make_pair(1.0, 1.0)
    };
    HomographicTransform<1, double> img2pln = threePoint1DHomography(img2pln_pairs);

    double const img_directrix1_proj = projector.projectionScalar(img_directrix1_pt);
    double const img_directrix2_proj = projector.projectionScalar(img_directrix2_pt);

    double const pln_directrix1_proj = toDouble(img2pln(toVec(img_directrix1_proj)));
    double const pln_directrix2_proj = toDouble(img2pln(toVec(img_directrix2_proj)));

    double const pln_middle_corrected_proj = 0.5 * (pln_directrix1_proj + pln_directrix2_proj);
    Vector3d const mdl_middle_corrected_pt(pln_x, pln_middle_corrected_proj, sample.fx);
    QPointF const img_middle_corrected_pt = toPoint(m_mdl2img(mdl_middle_corrected_pt));
    double const img_middle_corrected_proj = projector.projectionScalar(img_middle_corrected_pt);

    boost::array<std::pair<double, double>, 3> img2pln_pairs_corrected = {
        std::make_pair(img_directrix1_proj, 0.0),
        std::make_pair(img_middle_corrected_proj, pln_middle_corrected_proj),
        std::make_pair(img_directrix2_proj, 1.0)
    };
    HomographicTransform<1, double> img2pln_corrected = threePoint1DHomography(img2pln_pairs_corrected);

    double const img_pt_proj(projector.projectionScalar(img_pt));
    double const crv_y = img2pln_corrected(img_pt_proj);

    return QPointF(sample.arcLen, crv_y);
}

QPointF
CylindricalSurfaceDewarper::mapToWarpedSpace(QPointF const& crv_pt) const
{
    State state;
    Generatrix const gtx(mapGeneratrix(crv_pt.x(), state));
    return gtx.imgLine.pointAt(gtx.pln2img(crv_pt.y()));
}

HomographicTransform<2, double>
CylindricalSurfaceDewarper::calcPlnToImgHomography(
    std::vector<QPointF> const& img_directrix1,
    std::vector<QPointF> const& img_directrix2)
{
    boost::array<std::pair<QPointF, QPointF>, 4> pairs;
    pairs[0] = std::make_pair(QPointF(0, 0), img_directrix1.front());
    pairs[1] = std::make_pair(QPointF(1, 0), img_directrix1.back());
    pairs[2] = std::make_pair(QPointF(0, 1), img_directrix2.front());
    pairs[3] = std::make_pair(QPointF(1, 1), img_directrix2.back());

    return fourPoint2DHomography(pairs);
}

PerspectiveTransform
CylindricalSurfaceDewarper::calcMdlToImgTransform(
    HomographicTransform<2, double> const& pln2img,
    FovParams const& fov_params,
    FrameParams const& frame_params)
{
    Matrix<double, 3, 3> const& hmat = pln2img.mat();

    double const dx = frame_params.centerX();
    double const dy = frame_params.centerY();

    double const  h00 = hmat(0, 0) - hmat(2, 0) * dx;
    double const  h10 = hmat(1, 0) - hmat(2, 0) * dy;;
    double const& h20 = hmat(2, 0);

    double const  h01 = hmat(0, 1) - hmat(2, 1) * dx;
    double const  h11 = hmat(1, 1) - hmat(2, 1) * dy;;
    double const& h21 = hmat(2, 1);

    double const frame_size = std::max(
        frame_params.width(),
        frame_params.height()
    );

    double const F_inv_square = [h00, h10, h20, h01, h11, h21, &fov_params, frame_size]()
    {
        if (fov_params.mode() == MODE_AUTO)
        {
            double const F_inv_min = fov_params.fovMin() / frame_size;
            double const F_inv_max = fov_params.fovMax() / frame_size;

            double const F_inv_square_min = F_inv_min * F_inv_min;
            double const F_inv_square_max = F_inv_max * F_inv_max;

            double const F_inv_square = qBound(
                F_inv_square_min,
                -(h20 * h21) / (h00 * h01 + h10 * h11),
                F_inv_square_max
            );

            return F_inv_square;
        }
        else
        {
            double const F_inv = fov_params.fov() / frame_size;
            double const F_inv_square = F_inv * F_inv;

            return F_inv_square;
        }
    }();

    if (fov_params.mode() == MODE_AUTO)
        m_fov = frame_size * std::sqrt(F_inv_square);
    else
        m_fov = fov_params.fov();

    double const Sx_square = (h20 * h20) + (h00 * h00 + h10 * h10) * F_inv_square;
    double const Sy_square = (h21 * h21) + (h01 * h01 + h11 * h11) * F_inv_square;

    m_Sx = std::sqrt(Sx_square);
    m_Sy = std::sqrt(Sy_square);

    double const k = 1.0 / m_Sy;

    double const h02 = (h11 * h20 - h10 * h21) * k;
    double const h12 = (h00 * h21 - h01 * h20) * k;
    double const h22 = (h01 * h10 - h00 * h11) * k * F_inv_square;

    Matrix<double, 3, 1> const hvec(
        h02 + h22 * dx,
        h12 + h22 * dy,
        h22
    );

    return PerspectiveTransform(hmat, hvec);
}

void
CylindricalSurfaceDewarper::initArcLengthMapper(
    std::vector<QPointF> const& img_directrix1,
    std::vector<QPointF> const& img_directrix2)
{
    // TODO use bend params
    Directrix::Place const place1(m_mdl2img, img_directrix1, 0.0);
    Directrix::Place const place2(m_mdl2img, img_directrix2, 1.0);
    Directrix::Place const& best_place =
        place1.quality() > place2.quality()
        ? place1
        : place2;

    double const min_quality = 1e-3;
    Directrix::Plane const plane =
        best_place.quality() < min_quality
        ? best_place.createRotatedPlane(min_quality)
        : best_place.createPlane();

    Directrix::Profile profile(plane);

    double prev_x = NumericTraits<double>::min();
    for (QPointF const& point : profile.points())
    {
        if (point.x() <= prev_x)
        {
            continue;
        }

        m_arcLengthMapper.addSample(point.x(), point.y());

        prev_x = point.x();
    }

    // Needs to go before normalizeRange().
    m_directrixArcLength = m_arcLengthMapper.totalArcLength();

    // Scale arc lengths to the range of [0, 1].
    m_arcLengthMapper.normalizeRange(1);
}

} // namespace dewarping
