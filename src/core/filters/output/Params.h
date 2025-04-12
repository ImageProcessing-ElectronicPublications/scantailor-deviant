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

#ifndef OUTPUT_PARAMS_H_
#define OUTPUT_PARAMS_H_

#include "Dpi.h"
#include "ColorParams.h"
#include "DespeckleLevel.h"
#include "RegenParams.h"

class QDomDocument;
class QDomElement;

namespace output
{
enum ColorParamsApplyFilter {
    CopyMode = 1,
    CopyThreshold = 2,
    CopyForegroundThreshold = 4,
    CopyAllThresholds = CopyThreshold | CopyForegroundThreshold,
    CopyAll = CopyMode | CopyThreshold | CopyForegroundThreshold
};

class Params: public RegenParams
{
public:
    Params();

    Params(QDomElement const& el);

    Dpi const& outputDpi() const
    {
        return m_dpi;
    }

    void setOutputDpi(Dpi const& dpi)
    {
        m_dpi = dpi;
    }

    ColorParams& colorParams()
    {
        return m_colorParams;
    }

    ColorParams const& colorParams() const
    {
        return m_colorParams;
    }

    void setColorParams(ColorParams const& params, ColorParamsApplyFilter const& filter = CopyAll);

    DespeckleLevel despeckleLevel() const
    {
        return m_despeckleLevel;
    }

    void setDespeckleLevel(DespeckleLevel level)
    {
        m_despeckleLevel = level;
    }

//    QString const & TiffCompression() const
//    {
//        return m_TiffCompression;
//    }

//    void setTiffCompression(QString const& compression)
//    {
//        m_TiffCompression = compression;
//    }

    QDomElement toXml(QDomDocument& doc, QString const& name) const;
private:
    static ColorParams::ColorMode parseColorMode(QString const& str);

    static QString formatColorMode(ColorParams::ColorMode mode);

    ColorParams m_colorParams;

    Dpi m_dpi;
    DespeckleLevel m_despeckleLevel;
//    QString m_TiffCompression;
};

} // namespace output

#endif
