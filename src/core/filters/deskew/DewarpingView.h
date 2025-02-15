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

#ifndef DESKEW_DEWARPING_VIEW_H_
#define DESKEW_DEWARPING_VIEW_H_

#include "ImageViewBase.h"
#include "InteractionHandler.h"
#include "InteractiveXSpline.h"
#include "ImageTransformation.h"
#include "DragHandler.h"
#include "ZoomHandler.h"
#include "dewarping/DistortionModel.h"
#include "dewarping/FovParams.h"
#include "dewarping/FrameParams.h"
#include "dewarping/BendParams.h"

namespace spfit
{
class PolylineModelShape;
}

namespace deskew
{

class DewarpingView : public ImageViewBase, protected InteractionHandler
{
    Q_OBJECT
public:
    DewarpingView(
        QImage const& image, QImage const& downscaled_image,
        ImageTransformation const& xform,
        dewarping::DistortionModel const& distortion_model,
        dewarping::FovParams const& fov_params,
        dewarping::FrameParams const& frame_params,
        dewarping::BendParams const& bend_params,
        bool fixed_number_of_control_points);

    virtual ~DewarpingView();
signals:
    void distortionModelChanged(dewarping::DistortionModel const& model);
public slots:
    void fovParamsChanged(dewarping::FovParams const& fov_params);

    void frameParamsChanged(dewarping::FrameParams const& frame_params);

    void bendParamsChanged(dewarping::BendParams const& bend_params);
protected:
    virtual void onPaint(QPainter& painter, InteractionState const& interaction);
private:
    static void initNewSpline(XSpline& spline, QPointF const& p1, QPointF const& p2);

    static void fitSpline(XSpline& spline, std::vector<QPointF> const& polyline);

    static void curvatureAwareControlPointPositioning(
        XSpline& spline, spfit::PolylineModelShape const& model_shape);

    void paintXSpline(
        QPainter& painter, InteractionState const& interaction,
        InteractiveXSpline const& ispline);

    void curveModified(int curve_idx);

    void dragFinished();

    QPointF sourceToWidget(QPointF const& pt) const;

    QPointF widgetToSource(QPointF const& pt) const;

    dewarping::DistortionModel m_distortionModel;
    dewarping::FovParams m_fovParams;
    dewarping::FrameParams m_frameParams;
    dewarping::BendParams m_bendParams;
    InteractiveXSpline m_topSpline;
    InteractiveXSpline m_bottomSpline;
    DragHandler m_dragHandler;
    ZoomHandler m_zoomHandler;
};

} // namespace deskew

#endif
