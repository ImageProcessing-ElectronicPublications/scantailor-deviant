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
#include <algorithm>

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
    : m_left(defaultHMarginValue())
    , m_right(defaultHMarginValue())
    , m_top(defaultVMarginValue())
    , m_bottom(defaultVMarginValue())
    , m_maxPixelScale(defaultPixelScaleValue())
{
}

MarginsParams::MarginsParams(QDomElement const& el)
    : m_left(
        el.hasAttribute(str::LEFT) ?
        el.attribute(str::LEFT).toDouble() :
        defaultHMarginValue()
      )
    , m_right(
        el.hasAttribute(str::RIGHT) ?
        el.attribute(str::RIGHT).toDouble() :
        defaultHMarginValue()
      )
    , m_top(
        el.hasAttribute(str::TOP) ?
        el.attribute(str::TOP).toDouble() :
        defaultVMarginValue()
      )
    , m_bottom(
        el.hasAttribute(str::BOTTOM) ?
        el.attribute(str::BOTTOM).toDouble() :
        defaultVMarginValue()
      )
    , m_maxPixelScale(
        el.hasAttribute(str::MAX_PIXEL_SCALE) ?
        el.attribute(str::MAX_PIXEL_SCALE).toDouble() :
        defaultPixelScaleValue()
      )
{
    m_left = qBound(minHMarginValue(), m_left, maxHMarginValue());
    m_right = qBound(minHMarginValue(), m_right, maxHMarginValue());
    m_top = qBound(minVMarginValue(), m_top, maxVMarginValue());
    m_bottom = qBound(minVMarginValue(), m_bottom, maxVMarginValue());
    m_maxPixelScale = qBound(minPixelScaleValue(), m_maxPixelScale, maxPixelScaleValue());
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
