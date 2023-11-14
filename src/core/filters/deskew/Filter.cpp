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

#include "Filter.h"
#include "OptionsWidget.h"
#include "Task.h"
#include "CacheDrivenTask.h"
#include "Settings.h"
#include "AbstractRelinker.h"
#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include "CommandLine.h"

namespace deskew
{

Filter::Filter(PageSelectionAccessor const& page_selection_accessor)
    : m_ptrSettings(new Settings)
{
    if (CommandLine::get().isGui())
    {
        m_ptrOptionsWidget.reset(new OptionsWidget(m_ptrSettings, page_selection_accessor));
    }
}

Filter::~Filter()
{
}

QString
Filter::getName() const
{
    return QCoreApplication::translate("deskew::Filter", "Deskew");
}

PageView
Filter::getView() const
{
    return PAGE_VIEW;
}

void
Filter::performRelinking(AbstractRelinker const& relinker)
{
}

void
Filter::preUpdateUI(FilterUiInterface* const ui, PageId const& page_id)
{
}


QDomElement
Filter::saveSettings(ProjectWriter const& writer, QDomDocument& doc) const
{
    QDomElement filter_el(doc.createElement("deskew"));
    return filter_el;
}

void
Filter::loadSettings(ProjectReader const& reader, QDomElement const& filters_el)
{
}

IntrusivePtr<Task>
Filter::createTask(
    PageId const& page_id,
    IntrusivePtr<select_content::Task> const& next_task,
    bool const batch_processing, bool const debug)
{
    return IntrusivePtr<Task>(
        new Task(
            IntrusivePtr<Filter>(this),
            next_task, page_id, batch_processing
        )
    );
}

IntrusivePtr<CacheDrivenTask>
Filter::createCacheDrivenTask(
    IntrusivePtr<select_content::CacheDrivenTask> const& next_task)
{
    return IntrusivePtr<CacheDrivenTask>(
        new CacheDrivenTask(next_task)
    );
}

} // namespace deskew
