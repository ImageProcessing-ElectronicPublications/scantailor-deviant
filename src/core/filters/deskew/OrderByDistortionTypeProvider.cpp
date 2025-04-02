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

#include "OrderByDistortionTypeProvider.h"
#include "Params.h"
#include "PageId.h"
#include <QString>
#include <QObject>
#include <memory>
#include <assert.h>

namespace deskew
{

OrderByDistortionTypeProvider::OrderByDistortionTypeProvider(IntrusivePtr<Settings> const& settings)
    : m_ptrSettings(settings)
{
}

bool
OrderByDistortionTypeProvider::precedes(
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

    DistortionType lhs_type = DistortionType::NONE;
    if (lhs_params)
    {
        lhs_type = lhs_params->distortionType();
    }
    DistortionType rhs_type = DistortionType::NONE;
    if (rhs_params)
    {
        rhs_type = rhs_params->distortionType();
    }

    if (lhs_type == rhs_type)
    {
        return (lhs_page < rhs_page);
    }
    else
    {
        return (lhs_type < rhs_type);
    }
}

static QString
distortionType2String(DistortionType type)
{
    switch (type) {
    case DistortionType::NONE:
        return QObject::tr("none");
    case DistortionType::ROTATION:
        return QObject::tr("rotation");
    case DistortionType::PERSPECTIVE:
        return QObject::tr("perspective");
    case DistortionType::WARP:
        return QObject::tr("warp");
    default:
        return QObject::tr("unknown");
    }
}

QString
OrderByDistortionTypeProvider::hint(PageId const& page) const
{
    QString res(QObject::tr("Distortion type: %1"));

    std::unique_ptr<Params> const page_params = m_ptrSettings->getPageParams(page);
    if (page_params)
    {
        return res.arg(distortionType2String(page_params->distortionType()));
    }
    else
    {
        return res.arg(QObject::tr("unknown"));
    }
}

} // namespace deskew
