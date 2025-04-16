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

#ifndef THUMBNAIL_MAKER_BASE_H_
#define THUMBNAIL_MAKER_BASE_H_

#include <AbstractThumbnailMaker.h>

class ThumbnailMakerBase: public AbstractThumbnailMaker
{
    virtual QImage makeThumbnail(QImage const& image, QSize const& max_thumb_size) const;

    virtual std::unique_ptr<AbstractThumbnailMaker> clone() const;
};

#endif
