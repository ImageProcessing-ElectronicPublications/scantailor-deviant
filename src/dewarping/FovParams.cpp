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

#include "FovParams.h"
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
static char const FOV_MIN[] = "fov_min";
static char const FOV[] = "fov";
static char const FOV_MAX[] = "fov_max";
}

FovParams::FovParams()
    : m_mode(MODE_AUTO)
    , m_fovMin(0.2)
    , m_fov(1.5)
    , m_fovMax(2.0)
{
}

FovParams::FovParams(QDomElement const& el)
    : m_mode(el.attribute(str::MODE) == QLatin1String("manual") ? MODE_MANUAL : MODE_AUTO)
    , m_fovMin(el.attribute(str::FOV_MIN).toDouble())
    , m_fov(el.attribute(str::FOV).toDouble())
    , m_fovMax(el.attribute(str::FOV_MAX).toDouble())
{
}

QDomElement
FovParams::toXml(QDomDocument& doc, QString const& name) const
{
    XmlMarshaller marshaller(doc);

    QDomElement el(doc.createElement(name));
    el.setAttribute(str::MODE, m_mode == MODE_MANUAL ? "manual" : "auto");
    el.setAttribute(str::FOV_MIN, m_fovMin);
    el.setAttribute(str::FOV, m_fov);
    el.setAttribute(str::FOV_MAX, m_fovMax);
    return el;
}

} // namespace dewarping
