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

#ifndef DESKEW_FRAME_PARAMS_H_
#define DESKEW_FRAME_PARAMS_H_

#include "AutoManualMode.h"

class QDomDocument;
class QDomElement;
class QString;
class QRectF;

namespace dewarping
{

class FrameParams
{
public:
    FrameParams();

    FrameParams(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    AutoManualMode mode() const
    {
        return m_mode;
    }

    void setMode(AutoManualMode mode)
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

    double centerX() const
    {
        return m_centerX;
    }

    void setCenterX(double center_x)
    {
        m_centerX = center_x;
    }

    double centerY() const
    {
        return m_centerY;
    }

    void setCenterY(double center_y)
    {
        m_centerY = center_y;
    }

    void update(QRectF const& image_rect);

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

    static double minCenterValue()
    {
        return -32768;
    }

    static double maxCenterValue()
    {
        return 32768;
    }

    static double defaultCenterValue()
    {
        return 512;
    }
private:
    AutoManualMode m_mode;
    double m_width;
    double m_height;
    double m_centerX;
    double m_centerY;
};

} // namespace dewarping

#endif
