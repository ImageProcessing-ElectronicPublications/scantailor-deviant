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

#ifndef DESKEW_THUMBNAILVERSIONGENERATOR_H
#define DESKEW_THUMBNAILVERSIONGENERATOR_H

#include "PageId.h"
#include "DistortionType.h"

class QString;

namespace deskew
{

class ThumbnailVersionGenerator
{
public:
    ThumbnailVersionGenerator(
        PageId::SubPage sub_page,
        DistortionType::Type distortion_type);

    QString generate();

private:
    PageId::SubPage m_subPage;
    DistortionType::Type m_distortionType;
};

} // namespace deskew

#endif
