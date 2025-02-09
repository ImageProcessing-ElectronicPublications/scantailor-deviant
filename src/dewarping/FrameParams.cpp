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

#include "FrameParams.h"
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
static char const WIDTH[] = "width";
static char const HEIGHT[] = "height";
static char const CENTER_X[] = "center_x";
static char const CENTER_Y[] = "center_y";
}

FrameParams::FrameParams()
    : m_mode(MODE_AUTO)
    , m_width(1024)
    , m_height(1024)
    , m_centerX(0.5)
    , m_centerY(0.5)
{
}

FrameParams::FrameParams(QDomElement const& el)
    : m_mode(el.attribute(str::MODE) == QLatin1String("manual") ? MODE_MANUAL : MODE_AUTO)
    , m_width(el.attribute(str::WIDTH).toDouble())
    , m_height(el.attribute(str::HEIGHT).toDouble())
    , m_centerX(el.attribute(str::CENTER_X).toDouble())
    , m_centerY(el.attribute(str::CENTER_Y).toDouble())
{
}

QDomElement
FrameParams::toXml(QDomDocument& doc, QString const& name) const
{
   XmlMarshaller marshaller(doc);

   QDomElement el(doc.createElement(name));
   el.setAttribute(str::MODE, m_mode == MODE_MANUAL ? "manual" : "auto");
   el.setAttribute(str::WIDTH, m_width);
   el.setAttribute(str::HEIGHT, m_height);
   el.setAttribute(str::CENTER_X, m_centerX);
   el.setAttribute(str::CENTER_Y, m_centerY);
   return el;
}

} // namespace dewarping
