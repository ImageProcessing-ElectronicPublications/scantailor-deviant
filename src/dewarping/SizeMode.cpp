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

#include "SizeMode.h"
#include <QString>

namespace dewarping
{

namespace str
{
static char const BY_AREA[] = "by-area";
static char const FIT[] = "fit";
static char const STRETCH[] = "stretch";
static char const BY_DISTANCE[] = "by-distance";
}

SizeMode::SizeMode(Mode mode)
    : m_mode(mode)
{
}

SizeMode::SizeMode(QString const& string)
{
    if (string == QLatin1String(str::BY_DISTANCE))
        m_mode = BY_DISTANCE;
    else if (string == QLatin1String(str::STRETCH))
        m_mode = STRETCH;
    else if (string == QLatin1String(str::FIT))
        m_mode = FIT;
    else
        m_mode = BY_AREA;
}

QString
SizeMode::toString() const
{
    char const* s = str::BY_AREA;;
    switch (m_mode)
    {
    case BY_AREA:
        s = str::BY_AREA;
        break;
    case FIT:
        s = str::FIT;
        break;
    case STRETCH:
        s = str::STRETCH;
        break;
    case BY_DISTANCE:
        s = str::BY_DISTANCE;
        break;
    }
    return QLatin1String(s);
}

} // namespace dewarping
