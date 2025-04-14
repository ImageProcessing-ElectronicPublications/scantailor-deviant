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

#ifndef DESKEW_FOV_PARAMS_H_
#define DESKEW_FOV_PARAMS_H_

#include "AutoManualMode.h"

class QDomDocument;
class QDomElement;
class QString;

namespace dewarping
{

class FovParams
{
public:
    FovParams();

    FovParams(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    AutoManualMode mode() const
    {
        return m_mode;
    }

    void setMode(AutoManualMode mode)
    {
        m_mode = mode;
    }

    double fovMin() const
    {
        return m_fovMin;
    }

    void setFovMin(double fov_min)
    {
        m_fovMin = fov_min;
    }

    double fov() const
    {
        return m_fov;
    }

    void setFov(double fov)
    {
        m_fov = fov;
    }

    double fovMax() const
    {
        return m_fovMax;
    }

    void setFovMax(double fov_max)
    {
        m_fovMax = fov_max;
    }

    void update(double fov);

    static double minValue()
    {
        return 0.001;
    }

    static double maxValue()
    {
        return 10.0;
    }

    static double defaultMinValue()
    {
        return 0.2;
    }

    static double defaultValue()
    {
        return 1.5;
    }

    static double defaultMaxValue()
    {
        return 2.0;
    }
private:
    AutoManualMode m_mode;
    double m_fovMin;
    double m_fov;
    double m_fovMax;
};

} // namespace dewarping

#endif
