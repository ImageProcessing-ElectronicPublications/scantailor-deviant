/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>
    Copyright (C)  Vadim Kuznetsov ()DikBSD <dikbsd@gmail.com>

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

#ifndef PAGE_SPLIT_ORDER_BY_SOURCE_COLOR_PROVIDER_H_
#define PAGE_SPLIT_ORDER_BY_SOURCE_COLOR_PROVIDER_H_

#include <QObject>
#include "Settings.h"
#include "IntrusivePtr.h"
#include "ProjectPages.h"
#include "PageSequence.h"
#include "PageOrderProvider.h"

namespace output
{

class OrderBySourceColor : public QObject, public PageOrderProvider
{
    Q_OBJECT
public:
    OrderBySourceColor(IntrusivePtr<Settings> const& settings, IntrusivePtr<ProjectPages> const& pages);

    virtual bool precedes(
        PageId const& lhs_page, bool lhs_incomplete,
        PageId const& rhs_page, bool rhs_incomplete) const;

    virtual QString hint(PageId const& page) const;
private:
    IntrusivePtr<Settings> m_ptrSettings;
    IntrusivePtr<ProjectPages> m_pages;
public slots:
    void invalidate_metadata();
};

} // namespace page_split

#endif //PAGE_SPLIT_ORDER_BY_SOURCE_COLOR_PROVIDER_H_
