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

#ifndef OUTPUT_BLACK_WHITE_OPTIONS_H_
#define OUTPUT_BLACK_WHITE_OPTIONS_H_

class QString;
class QDomDocument;
class QDomElement;

namespace output
{

enum ThresholdFilter { OTSU, SAUVOLA, WOLF, GATOS };

class BlackWhiteOptions
{
public:
    BlackWhiteOptions();

    BlackWhiteOptions(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    ThresholdFilter thresholdMethod() const
    {
        return m_thresholdMethod;
    }

    void setThresholdMethod(ThresholdFilter val)
    {
        m_thresholdMethod = val;
    }

    int thresholdAdjustment() const;

    int thresholdOtsuAdjustment() const
    {
        return m_thresholdOtsuAdjustment;
    }

    void setThresholdOtsuAdjustment(int val)
    {
        m_thresholdOtsuAdjustment = val;
    }

    int thresholdSauvolaAdjustment() const
    {
        return m_thresholdSauvolaAdjustment;
    }

    void setThresholdSauvolaAdjustment(int val)
    {
        m_thresholdSauvolaAdjustment = val;
    }

    int thresholdSauvolaWindowSize() const
    {
        return m_thresholdSauvolaWindowSize;
    }

    void setThresholdSauvolaWindowSize(int val)
    {
        m_thresholdSauvolaWindowSize = val;
    }

    double thresholdSauvolaCoef() const
    {
        return m_thresholdSauvolaCoef;
    }

    void setThresholdSauvolaCoef(float val)
    {
        m_thresholdSauvolaCoef = val;
    }

    int thresholdWolfAdjustment() const
    {
        return m_thresholdWolfAdjustment;
    }

    void setThresholdWolfAdjustment(int val)
    {
        m_thresholdWolfAdjustment = val;
    }

    int thresholdWolfWindowSize() const
    {
        return m_thresholdWolfWindowSize;
    }

    void setThresholdWolfWindowSize(int val)
    {
        m_thresholdWolfWindowSize = val;
    }

    double thresholdWolfCoef() const
    {
        return m_thresholdWolfCoef;
    }

    void setThresholdWolfCoef(float val)
    {
        m_thresholdWolfCoef = val;
    }

    int thresholdGatosAdjustment() const
    {
        return m_thresholdGatosAdjustment;
    }

    void setThresholdGatosAdjustment(int val)
    {
        m_thresholdGatosAdjustment = val;
    }

    int thresholdGatosWindowSize() const
    {
        return m_thresholdGatosWindowSize;
    }

    void setThresholdGatosWindowSize(int val)
    {
        m_thresholdGatosWindowSize = val;
    }

    double thresholdGatosCoef() const
    {
        return m_thresholdGatosCoef;
    }

    void setThresholdGatosCoef(float val)
    {
        m_thresholdGatosCoef = val;
    }

    double thresholdGatosScale() const
    {
        return m_thresholdGatosScale;
    }

    void setThresholdGatosScale(float val)
    {
        m_thresholdGatosScale = val;
    }

    int thresholdForegroundAdjustment() const
    {
        return m_thresholdForegroundAdjustment;
    }

    void setThresholdForegroundAdjustment(int val)
    {
        m_thresholdForegroundAdjustment = val;
    }

    bool operator==(BlackWhiteOptions const& other) const;

    bool operator!=(BlackWhiteOptions const& other) const;
private:
    ThresholdFilter m_thresholdMethod;

    int m_thresholdOtsuAdjustment;

    int m_thresholdSauvolaAdjustment;
    int m_thresholdSauvolaWindowSize;
    double m_thresholdSauvolaCoef;

    int m_thresholdWolfAdjustment;
    int m_thresholdWolfWindowSize;
    double m_thresholdWolfCoef;

    int m_thresholdGatosAdjustment;
    int m_thresholdGatosWindowSize;
    double m_thresholdGatosCoef;
    double m_thresholdGatosScale;

    int m_thresholdForegroundAdjustment;

    static ThresholdFilter parseThresholdMethod(QString const& str);

    static QString formatThresholdMethod(ThresholdFilter type);
};

} // namespace output

#endif
