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

#include "MarginsParams.h"
#include "XmlMarshaller.h"
#include "XmlUnmarshaller.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace dewarping
{

namespace str
{
static char const LEFT[] = "left";
static char const RIGHT[] = "right";
static char const TOP[] = "top";
static char const BOTTOM[] = "bottom";
static char const MAX_PIXEL_SCALE[] = "max-pixel-scale";
}

MarginsParams::MarginsParams()
    : m_left(0.1)
    , m_right(0.1)
    , m_top(5.0)
    , m_bottom(5.0)
    , m_maxPixelScale(5.0)
{
}

MarginsParams::MarginsParams(QDomElement const& el)
    : m_left(el.attribute(str::LEFT).toDouble())
    , m_right(el.attribute(str::RIGHT).toDouble())
    , m_top(el.attribute(str::TOP).toDouble())
    , m_bottom(el.attribute(str::BOTTOM).toDouble())
    , m_maxPixelScale(el.attribute(str::MAX_PIXEL_SCALE).toDouble())
{
}

QDomElement
MarginsParams::toXml(QDomDocument& doc, QString const& name) const
{
    XmlMarshaller marshaller(doc);

    QDomElement el(doc.createElement(name));
    el.setAttribute(str::LEFT, m_left);
    el.setAttribute(str::RIGHT, m_right);
    el.setAttribute(str::TOP, m_top);
    el.setAttribute(str::BOTTOM, m_bottom);
    el.setAttribute(str::MAX_PIXEL_SCALE, m_maxPixelScale);
    return el;
}

} // namespace dewarping
