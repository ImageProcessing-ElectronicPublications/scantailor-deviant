/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#include "Thumbnail.h"
#include "AbstractThumbnailMaker.h"
#include "settings/globalstaticsettings.h"
#include <QRectF>
#include <QSizeF>
#include <QTransform>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>

#include <iostream>

namespace select_content
{

Thumbnail::Thumbnail(
    IntrusivePtr<ThumbnailPixmapCache> const& thumbnail_cache,
    std::unique_ptr<AbstractThumbnailMaker> thumb_maker,
    QSizeF const& max_size, ImageId const& image_id, QString const& version,
    ImageTransformation const& xform, QRectF const& content_rect, bool deviant)
    :   ThumbnailBase(
            thumbnail_cache, std::move(thumb_maker),
            max_size, image_id, version, xform),
        m_contentRect(content_rect), m_deviant(deviant)
{
}

void
Thumbnail::paintOverImage(
    QPainter& painter, QTransform const& image_to_display,
    QTransform const& thumb_to_display)
{
    Q_UNUSED(image_to_display);
    Q_UNUSED(thumb_to_display);

    if (m_contentRect.isNull()) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, false);

    QPen pen(GlobalStaticSettings::m_content_sel_content_color_pen);
    pen.setWidth(1);
    pen.setCosmetic(true);
    painter.setPen(pen);

    painter.setBrush(GlobalStaticSettings::m_content_sel_content_color);

    QRectF content_rect(virtToThumb().mapRect(m_contentRect));

    // Adjust to compensate for pen width.
    content_rect.adjust(-1, -1, 1, 1);

    // toRect() is necessary because we turn off antialiasing.
    // For some reason, if we let Qt round the coordinates,
    // the result is slightly different.
    painter.drawRect(content_rect.toAlignedRect());

    if (m_deviant) {
        paintDeviant(painter);
    }
}

} // namespace select_content
