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

#ifndef DESKEW_SIZE_PARAMS_H_
#define DESKEW_SIZE_PARAMS_H_

#include "SizeMode.h"

class QDomDocument;
class QDomElement;
class QString;

namespace dewarping
{

class SizeParams
{
public:
    SizeParams();

    SizeParams(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    SizeMode mode() const
    {
        return m_mode;
    }

    void setMode(SizeMode mode)
    {
        m_mode = mode;
    }

    double width() const
    {
        return m_width;
    }

    void setWidth(double width)
    {
        m_width = width;
    }

    double height() const
    {
        return m_height;
    }

    void setHeight(double height)
    {
        m_height = height;
    }

    double distance() const
    {
        return m_distance;
    }

    void setDistance(double distance)
    {
        m_distance = distance;
    }

    static double minSizeValue()
    {
        return 1;
    }

    static double maxSizeValue()
    {
        return 32768;
    }

    static double defaultSizeValue()
    {
        return 1024;
    }

    static double minDistanceValue()
    {
        return 1;
    }

    static double maxDistanceValue()
    {
        return 3276800;
    }

    static double defaultDistanceValue()
    {
        return 1024;
    }
private:
    SizeMode m_mode;
    double m_width;
    double m_height;
    double m_distance;
};

} // namespace dewarping

#endif
