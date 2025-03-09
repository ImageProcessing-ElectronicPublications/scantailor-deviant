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

#include "DewarpingThumbnail.h"
#include "dewarping/Curve.h"
#include "dewarping/DistortionModel.h"
#include "Utils.h"
#include <QTransform>
#include <QPainter>
#include <stdexcept>

using namespace dewarping;

namespace deskew
{

DewarpingThumbnail::DewarpingThumbnail(
	IntrusivePtr<ThumbnailPixmapCache> const& thumbnail_cache,
	QSizeF const& max_size, ImageId const& image_id,
    QString const& version,	ImageTransformation const& xform,
	std::vector<QPointF> const& top_curve,
	std::vector<QPointF> const& bottom_curve,
    dewarping::FovParams const& fov_params,
    dewarping::FrameParams const& frame_params,
    dewarping::BendParams const& bend_params)
	: ThumbnailBase(
        thumbnail_cache, max_size, 
        image_id, version, xform
      )
    , m_topCurve(top_curve)
    , m_bottomCurve(bottom_curve)
    , m_fovParams(fov_params)
    , m_frameParams(frame_params)
    , m_bendParams(bend_params)
{
    dewarping::DistortionModel distortion_model;
    distortion_model.setTopCurve(Curve(m_topCurve));
    distortion_model.setBottomCurve(Curve(m_bottomCurve));
    m_isValidModel = distortion_model.isValid();
}

void
DewarpingThumbnail::paintOverImage(
    QPainter& painter,
    QTransform const& image_to_display,
    QTransform const& thumb_to_display)
{
    Q_UNUSED(image_to_display);
    Q_UNUSED(thumb_to_display);

    if (!m_isValidModel)
    {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setTransform(imageXform().transform() * image_to_display);

    QPen pen(QColor(0, 0, 255, 150));
    pen.setWidthF(1.0);
    pen.setCosmetic(true);
    painter.setPen(pen);

    unsigned const num_horizontal_curves = 15;
    unsigned const num_vertical_lines = 10;
    std::vector<std::vector<QPointF>> horizontal_curves;
    std::vector<QLineF> vertical_lines;

    try
    {
        Utils::buildWarpVisualization(
            m_topCurve, m_bottomCurve,
            m_fovParams, m_frameParams, m_bendParams,
            num_horizontal_curves, num_vertical_lines,
            horizontal_curves, vertical_lines
        );

        for (auto const& curve : horizontal_curves)
        {
            painter.drawPolyline(curve.data(), curve.size());
        }

        for (auto const& line : vertical_lines)
        {
            painter.drawLine(line);
        }
    }
    catch (std::runtime_error const&)
    {
        // Invalid model?
    }
}

} // namespace deskew
