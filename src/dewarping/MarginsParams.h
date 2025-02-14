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

#ifndef DESKEW_MARGINS_PARAMS_H_
#define DESKEW_MARGINS_PARAMS_H_

class QDomDocument;
class QDomElement;
class QString;

namespace dewarping
{

class MarginsParams
{
public:
    MarginsParams();

    MarginsParams(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    double left() const
    {
        return m_left;
    }

    void setLeft(double left)
    {
        m_left = left;
    }

    double right() const
    {
        return m_right;
    }

    void setRight(double right)
    {
        m_right = right;
    }

    double top() const
    {
        return m_top;
    }

    void setTop(double top)
    {
        m_top = top;
    }

    double bottom() const
    {
        return m_bottom;
    }

    void setBottom(double bottom)
    {
        m_bottom = bottom;
    }

    double maxPixelScale() const
    {
        return m_maxPixelScale;
    }

    void setMaxPixelScale(double max_pixel_scale)
    {
        m_maxPixelScale = max_pixel_scale;
    }

    static double minHMarginValue()
    {
        return 0.0;
    }

    static double maxHMarginValue()
    {
        return 10.0;
    }

    static double defaultHMarginValue()
    {
        return 0.1;
    }

    static double minVMarginValue()
    {
        return 0.0;
    }

    static double maxVMarginValue()
    {
        return 10.0;
    }

    static double defaultVMarginValue()
    {
        return 5.0;
    }

    static double minPixelScaleValue()
    {
        return 1.0;
    }

    static double maxPixelScaleValue()
    {
        return 5.0;
    }

    static double defaultPixelScaleValue()
    {
        return 10.0;
    }
private:
    double m_left;
    double m_right;
    double m_top;
    double m_bottom;
    double m_maxPixelScale;
};

} // namespace dewarping

#endif
