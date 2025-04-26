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

#ifndef DEWARPING_DEWARPING_IMAGE_TRANSFORM_H_
#define DEWARPING_DEWARPING_IMAGE_TRANSFORM_H_

#include "CylindricalSurfaceDewarper.h"
#include "imageproc/AbstractImageTransform.h"
#include "SizeParams.h"
#include <QSize>
#include <QPolygonF>
#include <vector>

class QImage;
class QColor;
class QString;

namespace imageproc
{
class AffineImageTransform;
class AffineTransformedImage;
}

namespace dewarping
{

class FovParams;
class FrameParams;
class BendParams;

class DewarpingImageTransform : public imageproc::AbstractImageTransform
{
    // Member-wise copying is OK.
public:
    /**
     * @note The provided crop area in original image coordinates may be cropped further,
     *       as some areas of the image may be unsafe to dewarp.
     */
    DewarpingImageTransform(
        QSize const& orig_size,
        QPolygonF const& orig_crop_area,
        std::vector<QPointF> const& top_curve,
        std::vector<QPointF> const& bottom_curve,
        FovParams const& fov_params,
        FrameParams const& frame_params,
        BendParams const& bend_params,
        SizeParams const& size_params);

    virtual ~DewarpingImageTransform();

    virtual std::unique_ptr<AbstractImageTransform> clone() const;

    virtual bool isAffine() const
    {
        return false;
    }

    virtual QSize const& origSize() const
    {
        return m_origSize;
    }

    virtual QPolygonF const& origCropArea() const
    {
        return m_origCropArea;
    }

    virtual QPolygonF transformedCropArea() const;

    ImageSize imageSize() const;

    double fov() const
    {
        return m_dewarper.fov();
    }

    double bend() const
    {
        return m_dewarper.bend();
    }

    virtual QTransform scale(qreal xscale, qreal yscale);

    virtual DewarpingImageTransform scaled(qreal xscale, qreal yscale) const;

    virtual QImage materialize(QImage const& image,
                               QRect const& target_rect, QColor const& outside_color) const;

    virtual std::function<QPointF(QPointF const&)> forwardMapper() const;

    virtual std::function<QPointF(QPointF const&)> backwardMapper() const;
private:
    class ConstrainedCropAreaBuilder;

    void setupIntrinsicScale();

    QPolygonF constrainCropArea(QPolygonF const& orig_crop_area) const;

    std::pair<double, double> calcMinMaxDensities() const;

    QPointF postScale(QPointF const& pt) const;

    QSize m_origSize;
    QPolygonF m_origCropArea;
    std::vector<QPointF> m_topPolyline;
    std::vector<QPointF> m_bottomPolyline;
    SizeParams m_sizeParams;
    CylindricalSurfaceDewarper m_dewarper;

    /**
     * CylindricalSurfaceDewarper maps a curved quadrilateral into a unit square.
     * Two rounds of post scaling are then applied to map that unit square to its
     * final size. The first round is intrinsic scaling. Its parameters are derived
     * from the distortion model and its purpose is to match the pixel density inside
     * the curved and dewarped quadrilaterals. The intrinsic scale factors don't
     * participate in transform fingerprint calculation, as they are not free parameters
     * but are derived from the distortion model. Because the floating point values
     * in the distortion model are rounded when saving to XML, recalculated intrinsic
     * scale factors will be a bit off in a way that RoundingHasher (used in transform
     * fingerprint calculation) won't be able to compensate.
     */
    qreal m_intrinsicScaleX;
    qreal m_intrinsicScaleY;

    /**
     * Additional post-scaling is the result of client code calling scale().
     * The reason it's separate from m_intrinsicScale[XY] is that m_userScale[XY]
     * do participate in transform fingerprint calculation, while m_intrinsicScale[XY]
     * don't.
     */
    qreal m_userScaleX;
    qreal m_userScaleY;
};

} // namespace dewarping

#endif
