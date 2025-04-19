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

#include "ThumbnailMakerBase.h"
#include "imageproc/Scale.h"
#include "imageproc/GrayImage.h"
#include <QImage>
#include <QSize>

using namespace imageproc;

QImage
ThumbnailMakerBase::makeThumbnail(QImage const& image, QSize const& max_thumb_size) const
{
    if (image.width() < max_thumb_size.width() &&
        image.height() < max_thumb_size.height()) {
        return image;
    }

    QSize to_size(image.size());
    to_size.scale(max_thumb_size, Qt::KeepAspectRatio);

    if (image.format() == QImage::Format_Indexed8 && image.isGrayscale()) {
        // This will be faster than QImage::scale().
        return scaleToGray(GrayImage(image), to_size);
    }

    return image.scaled(
        to_size,
        Qt::KeepAspectRatio, Qt::SmoothTransformation
    );
}

std::unique_ptr<AbstractThumbnailMaker>
ThumbnailMakerBase::clone() const
{
    return std::unique_ptr<AbstractThumbnailMaker>(new ThumbnailMakerBase(*this));
}
