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
#include <QRectF>

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
    , m_width(defaultSizeValue())
    , m_height(defaultSizeValue())
    , m_centerX(defaultCenterValue())
    , m_centerY(defaultCenterValue())
{
}

FrameParams::FrameParams(QDomElement const& el)
    : m_mode(el.attribute(str::MODE) == QLatin1String("manual") ? MODE_MANUAL : MODE_AUTO)
    , m_width(
        el.hasAttribute(str::WIDTH) ?
        el.attribute(str::WIDTH).toDouble() :
        defaultSizeValue()
      )
    , m_height(
        el.hasAttribute(str::HEIGHT) ?
        el.attribute(str::HEIGHT).toDouble() :
        defaultSizeValue()
       )
    , m_centerX(
        el.hasAttribute(str::CENTER_X) ?
        el.attribute(str::CENTER_X).toDouble() :
        defaultCenterValue()
      )
    , m_centerY(
        el.hasAttribute(str::CENTER_Y) ?
        el.attribute(str::CENTER_Y).toDouble() :
        defaultCenterValue()
      )
{
    m_width = qBound(minSizeValue(), m_width, maxSizeValue());
    m_height = qBound(minSizeValue(), m_height, maxSizeValue());
    m_centerX = qBound(minCenterValue(), m_centerX, maxCenterValue());
    m_centerY = qBound(minCenterValue(), m_centerY, maxCenterValue());
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

void
FrameParams::maybeUpdate(QRectF const& image_rect)
{
    if (m_mode == MODE_AUTO)
    {
        m_width = image_rect.width();
        m_height = image_rect.height();
        m_centerX = image_rect.center().x();
        m_centerY = image_rect.center().y();
    }
}

} // namespace dewarping
