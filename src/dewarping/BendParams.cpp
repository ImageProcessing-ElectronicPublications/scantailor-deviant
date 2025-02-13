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

#include "BendParams.h"
#include "XmlMarshaller.h"
#include "XmlUnmarshaller.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace dewarping
{

namespace str
{
static char const MODE[] = "mode";
static char const BEND_MIN[] = "bend_min";
static char const BEND[] = "bend";
static char const BEND_MAX[] = "bend_max";
}

BendParams::BendParams()
    : m_mode(MODE_AUTO)
    , m_bendMin(-0.5)
    , m_bend(0.15)
    , m_bendMax(0.5)
{
}

BendParams::BendParams(AutoManualMode mode, double bend_min, double bend, double bend_max)
    : m_mode(mode)
    , m_bendMin(bend_min)
    , m_bend(bend)
    , m_bendMax(bend_max)
{
}

BendParams::BendParams(QDomElement const& el)
    : m_mode(el.attribute(str::MODE) == QLatin1String("manual") ? MODE_MANUAL : MODE_AUTO)
    , m_bendMin(el.attribute(str::BEND_MIN).toDouble())
    , m_bend(el.attribute(str::BEND).toDouble())
    , m_bendMax(el.attribute(str::BEND_MAX).toDouble())
{
}

QDomElement
BendParams::toXml(QDomDocument& doc, QString const& name) const
{
    XmlMarshaller marshaller(doc);

    QDomElement el(doc.createElement(name));
    el.setAttribute(str::MODE, m_mode == MODE_MANUAL ? "manual" : "auto");
    el.setAttribute(str::BEND_MIN, m_bendMin);
    el.setAttribute(str::BEND, m_bend);
    el.setAttribute(str::BEND_MAX, m_bendMax);
    return el;
}

} // namespace dewarping
