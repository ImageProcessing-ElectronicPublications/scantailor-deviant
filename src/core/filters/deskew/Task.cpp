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

#include "Task.h"
#include "Filter.h"
#include "OptionsWidget.h"
#include "FilterData.h"
#include "filters/select_content/Task.h"
#include "FilterUiInterface.h"
#include "ImageView.h"
#include "ImageTransformation.h"
#include <memory>

namespace deskew
{

class Filter;

/*======================== Task::NoDistortionUiUpdater =====================*/

class Task::NoDistortionUiUpdater : public FilterResult
{
public:
	NoDistortionUiUpdater(
        IntrusivePtr<Filter> const& filter,
		QImage const& image, PageId const& page_id,
		ImageTransformation const& xform,
		bool batch_processing);

    virtual void updateUI(FilterUiInterface* ui);

    virtual IntrusivePtr<AbstractFilter> filter()
    {
        return m_ptrFilter;
    }
private:
    IntrusivePtr<Filter> m_ptrFilter;
    QImage m_image;
    QImage m_downscaledImage;
    PageId m_pageId;
    ImageTransformation m_xform;
    bool m_batchProcessing;
};

/*=================================== Task =================================*/

Task::Task(
    IntrusivePtr<Filter> const& filter,
    IntrusivePtr<select_content::Task> const& next_task,
    PageId const& page_id, bool batch_processing)
    : m_ptrFilter(filter)
    , m_ptrNextTask(next_task)
    , m_pageId(page_id)
    , m_batchProcessing(batch_processing)
{

}

Task::~Task()
{
}

FilterResultPtr
Task::process(TaskStatus const& status, FilterData const& data)
{
    ImageTransformation new_xform(data.xform());

    if (m_ptrNextTask) {
        return m_ptrNextTask->process(status, FilterData(data, new_xform));
    }
    else {
        return FilterResultPtr(
            new NoDistortionUiUpdater(m_ptrFilter, data.origImage(), 
                m_pageId, new_xform, m_batchProcessing)
        );
    }
}

/*======================== Task::NoDistortionUiUpdater =====================*/

Task::NoDistortionUiUpdater::NoDistortionUiUpdater(
    IntrusivePtr<Filter> const& filter,
    QImage const& image, PageId const& page_id,
    ImageTransformation const& xform,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_image(image)
    , m_downscaledImage(ImageView::createDownscaledImage(image))
    , m_pageId(page_id)
    , m_xform(xform)
    , m_batchProcessing(batch_processing)
{
}

void
Task::NoDistortionUiUpdater::updateUI(FilterUiInterface* ui)
{
    OptionsWidget* const opt_widget = m_ptrFilter->optionsWidget();
    ui->setOptionsWidget(opt_widget, ui->KEEP_OWNERSHIP);

    ui->invalidateThumbnail(m_pageId);

    if (m_batchProcessing) {
        return;
    }

    ImageView* view = new ImageView(m_image, m_downscaledImage, m_xform);
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP);
}

} // namespace deskew
