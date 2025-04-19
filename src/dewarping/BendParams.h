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

#ifndef DESKEW_BEND_PARAMS_H_
#define DESKEW_BEND_PARAMS_H_

#include "AutoManualMode.h"

class QDomDocument;
class QDomElement;
class QString;

namespace dewarping
{

class BendParams
{
public:
    BendParams();

    BendParams(AutoManualMode mode, double bend_min, double bend, double bend_max);

    BendParams(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    AutoManualMode mode() const
    {
        return m_mode;
    }

    void setMode(AutoManualMode mode)
    {
        m_mode = mode;
    }

    double bendMin() const
    {
        return m_bendMin;
    }

    void setBendMin(double bend_min)
    {
        m_bendMin = bend_min;
    }

    double bend() const
    {
        return m_bend;
    }

    void setBend(double bend)
    {
        m_bend = bend;
    }

    double bendMax() const
    {
        return m_bendMax;
    }

    void setBendMax(double bend_max)
    {
        m_bendMax = bend_max;
    }

    static double minValue()
    {
        return -1.0;
    }

    static double maxValue()
    {
        return 1.0;
    }

    static double defaultMinValue()
    {
        return -0.5;
    }

    static double defaultValue()
    {
        return 0.15;
    }

    static double defaultMaxValue()
    {
        return 0.5;
    }
private:
    AutoManualMode m_mode;
    double m_bendMin;
    double m_bend;
    double m_bendMax;
};

} // namespace dewarping

#endif
