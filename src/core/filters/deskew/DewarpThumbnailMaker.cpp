/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2008  Joseph Artsimovich <joseph_a@mail.ru>

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

#include "DewarpThumbnailMaker.h"
#include "imageproc/Scale.h"
#include "imageproc/GrayImage.h"
#include <QImage>
#include <QSize>
#include <QRectF>
#include <QRect>
#include <algorithm>

using namespace imageproc;
using namespace dewarping;

DewarpThumbnailMaker::DewarpThumbnailMaker(dewarping::DewarpingImageTransform const& transform)
    : m_transform(transform)
{
}

QImage
DewarpThumbnailMaker::makeThumbnail(QImage const& image, QSize const& max_thumb_size) const
{
    QRectF const original_img_rect = m_transform.transformedCropArea().boundingRect();
    double const scale = std::min(
        max_thumb_size.width() / original_img_rect.width(),
        max_thumb_size.height() / original_img_rect.height()
    );
    DewarpingImageTransform const downscaled_transform = m_transform.scaled(scale, scale);
    QRectF const downscaled_img_rect = downscaled_transform.transformedCropArea().boundingRect();

    return downscaled_transform.materialize(
        image,
        QRect(
            downscaled_img_rect.left(),
            downscaled_img_rect.top(),
            downscaled_img_rect.width(),
            downscaled_img_rect.height()
        ),
        QColor(255,255,255, 0)
    ).convertToFormat(QImage::Format_ARGB32);
}

std::unique_ptr<AbstractThumbnailMaker>
DewarpThumbnailMaker::clone() const
{
    return std::unique_ptr<AbstractThumbnailMaker>(new DewarpThumbnailMaker(*this));
}
