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
static char const CALC_BY_AREA[] = "calc-by-area";
static char const FIT_WIDTH[] = "fit-width";
static char const FIT_HEIGHT[] = "fit-height";
static char const STRETCH_TO[] = "stretch-to";
static char const CALC_BY_DISTANCE[] = "calc-by-distance";
}

SizeMode::SizeMode(Mode mode)
    : m_mode(mode)
{
}

SizeMode::SizeMode(QString const& string)
{
    if (string == QLatin1String(str::CALC_BY_DISTANCE))
        m_mode = CALC_BY_DISTANCE;
    else if (string == QLatin1String(str::STRETCH_TO))
        m_mode = STRETCH_TO;
    else if (string == QLatin1String(str::FIT_HEIGHT))
        m_mode = FIT_HEIGHT;
    else if (string == QLatin1String(str::FIT_WIDTH))
        m_mode = FIT_WIDTH;
    else
        m_mode = CALC_BY_AREA;
}

QString
SizeMode::toString() const
{
    char const* s = str::CALC_BY_AREA;
    switch (m_mode)
    {
    case FIT_WIDTH:
        s = str::FIT_WIDTH;
        break;
    case FIT_HEIGHT:
        s = str::FIT_HEIGHT;
        break;
    case STRETCH_TO:
        s = str::STRETCH_TO;
        break;
    case CALC_BY_DISTANCE:
        s = str::CALC_BY_DISTANCE;
        break;
    }
    return QLatin1String(s);
}

} // namespace dewarping
