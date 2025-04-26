/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
	Copyright (C) Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#include "OrderByDewarpedWidthProvider.h"
#include "PageId.h"
#include "Params.h"
#include <QString>
#include <QObject>
#include <limits>
#include <memory>
#include <assert.h>

namespace deskew
{

OrderByDewarpedWidthProvider::OrderByDewarpedWidthProvider(IntrusivePtr<Settings> const& settings)
    : m_ptrSettings(settings)
{
}

static double getWidth(std::unique_ptr<Params> const& params)
{
    switch (params->distortionType())
    {
    case DistortionType::PERSPECTIVE:
    {
        Value const width = params->perspectiveParams().sizeParams().width();
        if(width.isValid())
            return width;
        else
            return std::numeric_limits<double>::max();
    }
    case DistortionType::WARP:
    {
        Value const width = params->dewarpingParams().sizeParams().width();
        if(width.isValid())
            return width;
        else
            return std::numeric_limits<double>::max();
    }
    default:
        return std::numeric_limits<double>::max();
    }
}

bool
OrderByDewarpedWidthProvider::precedes(
    PageId const& lhs_page, bool const lhs_incomplete,
    PageId const& rhs_page, bool const rhs_incomplete) const
{
    if (lhs_incomplete != rhs_incomplete) {
        // Pages with question mark go to the bottom.
        return rhs_incomplete;
    }
    else if (lhs_incomplete) {
        assert(rhs_incomplete);
        // Two pages with question marks are ordered naturally.
        return lhs_page < rhs_page;
    }

    assert(lhs_incomplete == false);
    assert(rhs_incomplete == false);

    std::unique_ptr<Params> const lhs_params = m_ptrSettings->getPageParams(lhs_page);
    std::unique_ptr<Params> const rhs_params = m_ptrSettings->getPageParams(rhs_page);

    double lhs_width = 0.0;
    if (lhs_params)
    {
        lhs_width = getWidth(lhs_params);
    }

    double rhs_width = 0.0;
    if (rhs_params)
    {
        rhs_width = getWidth(rhs_params);
    }

    if (lhs_width == rhs_width)
    {
        return (lhs_page < rhs_page);
    }
    else
    {
        return (lhs_width < rhs_width);
    }
}

QString
OrderByDewarpedWidthProvider::hint(PageId const& page) const
{
    QString res(QObject::tr("Width: %1"));
    std::unique_ptr<Params> const page_params = m_ptrSettings->getPageParams(page);
    if (page_params)
    {
        switch (page_params->distortionType())
        {
        case DistortionType::PERSPECTIVE:
        {
            Value const width = page_params->perspectiveParams().sizeParams().width();
            if(width.isValid())
                return res.arg(width, 0, 'f', 0);
            else
                return res.arg(QObject::tr("unknown"));
        }
        case DistortionType::WARP:
        {
            Value const width = page_params->dewarpingParams().sizeParams().width();
            if (width.isValid())
                return res.arg(width, 0, 'f', 0);
            else
                return res.arg(QObject::tr("unknown"));
        }
        default:
            return res.arg(QObject::tr("unknown"));
        }
    }
    else
    {
        return res.arg(QObject::tr("unknown"));
    }
}

} // namespace deskew
