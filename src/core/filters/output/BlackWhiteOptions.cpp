/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#include "BlackWhiteOptions.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include "settings/ini_keys.h"
#include "settings/globalstaticsettings.h"

namespace output
{

BlackWhiteOptions::BlackWhiteOptions()
    : m_thresholdMethod(OTSU)

    , m_thresholdOtsuAdjustment(GlobalStaticSettings::m_binrization_threshold_control_default)

    , m_thresholdSauvolaAdjustment(GlobalStaticSettings::m_binrization_threshold_control_default)
    , m_thresholdSauvolaWindowSize(200)
    , m_thresholdSauvolaCoef(0.3)

    , m_thresholdWolfAdjustment(GlobalStaticSettings::m_binrization_threshold_control_default)
    , m_thresholdWolfWindowSize(200)
    , m_thresholdWolfCoef(0.3)

    , m_thresholdGatosAdjustment(GlobalStaticSettings::m_binrization_threshold_control_default)
    , m_thresholdGatosWindowSize(200)
    , m_thresholdGatosCoef(0.3)
    , m_thresholdGatosScale(0.6)

    , m_thresholdForegroundAdjustment(GlobalStaticSettings::m_binrization_threshold_control_default)
{
}

BlackWhiteOptions::BlackWhiteOptions(QDomElement const& el)
    : m_thresholdMethod(parseThresholdMethod(el.attribute("thresholdMethod")))

    , m_thresholdOtsuAdjustment(el.attribute("thresholdOtsuAdj").toInt())

    , m_thresholdSauvolaAdjustment(el.attribute("thresholdSauvolaAdj").toInt())
    , m_thresholdSauvolaWindowSize(el.attribute("thresholdSauvolaWinSize").toInt())
    , m_thresholdSauvolaCoef(el.attribute("thresholdSauvolaCoef").toDouble())

    , m_thresholdWolfAdjustment(el.attribute("thresholdWolfAdj").toInt())
    , m_thresholdWolfWindowSize(el.attribute("thresholdWolfWinSize").toInt())
    , m_thresholdWolfCoef(el.attribute("thresholdWolfCoef").toDouble())

    , m_thresholdGatosAdjustment(el.attribute("thresholdGatosAdj").toInt())
    , m_thresholdGatosWindowSize(el.attribute("thresholdGatosWinSize").toInt())
    , m_thresholdGatosCoef(el.attribute("thresholdGatosCoef").toDouble())
    , m_thresholdGatosScale(el.attribute("thresholdGatosScale").toDouble())

    , m_thresholdForegroundAdjustment(el.attribute("thresholdForegAdj").toInt())
{
    if (m_thresholdSauvolaWindowSize < 1 || m_thresholdSauvolaWindowSize > 9999)
    {
        m_thresholdSauvolaWindowSize = 200;
    }
    if (m_thresholdSauvolaCoef < 0.01 || m_thresholdSauvolaCoef > 9.99)
    {
        m_thresholdSauvolaCoef = 0.3;
    }

    if (m_thresholdWolfWindowSize < 1 || m_thresholdWolfWindowSize > 9999)
    {
        m_thresholdWolfWindowSize = 200;
    }
    if (m_thresholdWolfCoef < 0.01 || m_thresholdWolfCoef > 9.99)
    {
        m_thresholdWolfCoef = 0.3;
    }

    if (m_thresholdGatosWindowSize < 1 || m_thresholdGatosWindowSize > 9999)
    {
        m_thresholdGatosWindowSize = 200;
    }
    if (m_thresholdGatosCoef < 0.01 || m_thresholdGatosCoef > 9.99)
    {
        m_thresholdGatosCoef = 0.3;
    }
    if (m_thresholdGatosScale < 0.01 || m_thresholdGatosScale > 1.0)
    {
        m_thresholdGatosScale = 0.6;
    }
}

QDomElement
BlackWhiteOptions::toXml(QDomDocument& doc, QString const& name) const
{
    QDomElement el(doc.createElement(name));
    el.setAttribute("thresholdMethod", formatThresholdMethod(m_thresholdMethod));

    el.setAttribute("thresholdOtsuAdj", m_thresholdOtsuAdjustment);

    el.setAttribute("thresholdSauvolaAdj", m_thresholdSauvolaAdjustment);
    el.setAttribute("thresholdSauvolaWinSize", m_thresholdSauvolaWindowSize);
    el.setAttribute("thresholdSauvolaCoef", m_thresholdSauvolaCoef);

    el.setAttribute("thresholdWolfAdj", m_thresholdWolfAdjustment);
    el.setAttribute("thresholdWolfWinSize", m_thresholdWolfWindowSize);
    el.setAttribute("thresholdWolfCoef", m_thresholdWolfCoef);

    el.setAttribute("thresholdGatosAdj", m_thresholdGatosAdjustment);
    el.setAttribute("thresholdGatosWinSize", m_thresholdGatosWindowSize);
    el.setAttribute("thresholdGatosCoef", m_thresholdGatosCoef);
    el.setAttribute("thresholdGatosScale", m_thresholdGatosScale);

    el.setAttribute("thresholdForegAdj", m_thresholdForegroundAdjustment);

    return el;
}

int
BlackWhiteOptions::thresholdAdjustment() const
{
    switch (m_thresholdMethod)
    {
    case OTSU:
        return m_thresholdOtsuAdjustment;
    case SAUVOLA:
        return m_thresholdSauvolaAdjustment;
    case WOLF:
        return m_thresholdSauvolaAdjustment;
    case GATOS:
        return m_thresholdGatosAdjustment;
    default:
        assert(!"Unreachable");
        return m_thresholdOtsuAdjustment;
    }
}

bool
BlackWhiteOptions::operator==(BlackWhiteOptions const& other) const
{
    if (m_thresholdMethod != other.m_thresholdMethod)
    {
        return false;
    }

    if (m_thresholdOtsuAdjustment != other.m_thresholdOtsuAdjustment) {
        return false;
    }

    if (m_thresholdSauvolaAdjustment != other.m_thresholdSauvolaAdjustment) {
        return false;
    }
    if (m_thresholdSauvolaWindowSize != other.m_thresholdSauvolaWindowSize)
    {
        return false;
    }
    if (m_thresholdSauvolaCoef != other.m_thresholdSauvolaCoef)
    {
        return false;
    }

    if (m_thresholdWolfAdjustment != other.m_thresholdWolfAdjustment) {
        return false;
    }
    if (m_thresholdWolfWindowSize != other.m_thresholdWolfWindowSize)
    {
        return false;
    }
    if (m_thresholdWolfCoef != other.m_thresholdWolfCoef)
    {
        return false;
    }

    if (m_thresholdGatosAdjustment != other.m_thresholdGatosAdjustment) {
        return false;
    }
    if (m_thresholdGatosWindowSize != other.m_thresholdGatosWindowSize)
    {
        return false;
    }
    if (m_thresholdGatosCoef != other.m_thresholdGatosCoef)
    {
        return false;
    }
    if (m_thresholdGatosScale != other.m_thresholdGatosScale)
    {
        return false;
    }

    if (m_thresholdForegroundAdjustment != other.m_thresholdForegroundAdjustment) {
        return false;
    }

    return true;
}

bool
BlackWhiteOptions::operator!=(BlackWhiteOptions const& other) const
{
    return !(*this == other);
}

ThresholdFilter
BlackWhiteOptions::parseThresholdMethod(QString const& str)
{
    if (str == "sauvola")
    {
        return SAUVOLA;
    }
    else if (str == "wolf")
    {
        return WOLF;
    }
    else if (str == "gatos")
    {
        return GATOS;
    }
    else
    {
        return OTSU;
    }
}

QString
BlackWhiteOptions::formatThresholdMethod(ThresholdFilter type)
{
    QString str = "";
    switch (type)
    {
    case OTSU:
        str = "otsu";
        break;
    case SAUVOLA:
        str = "sauvola";
        break;
    case WOLF:
        str = "wolf";
        break;
    case GATOS:
        str = "gatos";
        break;
    }
    return str;
}
} // namespace output
