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

#include "ThumbnailVersionGenerator.h"
#include <QString>

namespace deskew
{

ThumbnailVersionGenerator::ThumbnailVersionGenerator(
    PageId::SubPage sub_page,
    DistortionType::Type distortion_type)
    : m_subPage(sub_page), m_distortionType(distortion_type)
{
}

QString
ThumbnailVersionGenerator::generate()
{
    switch (m_distortionType)
    {
    case DistortionType::PERSPECTIVE:
    case DistortionType::WARP:
        switch (m_subPage)
        {
        case PageId::SINGLE_PAGE:
            return QString('S');
        case PageId::LEFT_PAGE:
            return QString('L');
        case PageId::RIGHT_PAGE:
            return QString('R');
        }
    default:
        return QString();
    }
}

} // namespace deskew
