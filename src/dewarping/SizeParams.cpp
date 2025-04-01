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

#include "SizeParams.h"
#include "ImageSize.h"
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
    static char const DISTANCE[] = "distance";
}

SizeParams::SizeParams()
    : m_mode(SizeMode::BY_AREA)
    , m_width(defaultSizeValue())
    , m_height(defaultSizeValue())
    , m_distance(defaultDistanceValue())
{
}

SizeParams::SizeParams(QDomElement const& el)
    : m_mode(el.attribute(str::MODE))
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
    , m_distance(
        el.hasAttribute(str::DISTANCE) ?
        el.attribute(str::DISTANCE).toDouble() :
        defaultDistanceValue()
      )
{
}

QDomElement
SizeParams::toXml(QDomDocument& doc, QString const& name) const
{
    XmlMarshaller marshaller(doc);

    QDomElement el(doc.createElement(name));
    el.setAttribute(str::MODE, m_mode.toString());
    el.setAttribute(str::WIDTH, m_width);
    el.setAttribute(str::HEIGHT, m_height);
    el.setAttribute(str::DISTANCE, m_distance);
    return el;
}

void
SizeParams::update(ImageSize const& image_size)
{
    switch (m_mode)
    {
    case SizeMode::BY_AREA:
        m_width = image_size.width;
        m_height = image_size.height;
        m_distance = image_size.distance;
        break;
    case SizeMode::FIT:
        m_distance = image_size.distance;
        break;
    case SizeMode::STRETCH:
        m_distance = image_size.distance;
        break;
    case SizeMode::BY_DISTANCE:
        m_width = image_size.width;
        m_height = image_size.height;
        break;
    default:
        assert(!"Unreachable");
        break;
    }
}

} // namespace dewarping
