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

namespace output
{

Thumbnail::Thumbnail(
    IntrusivePtr<ThumbnailPixmapCache> const& thumbnail_cache,
    std::unique_ptr<AbstractThumbnailMaker> thumb_maker,
    QSizeF const& max_size, ImageId const& image_id,
    QString const& version, ImageTransformation const& xform)
    :   ThumbnailBase(
            thumbnail_cache, std::move(thumb_maker),
            max_size, image_id, version, xform)
{
}

} // namespace output
