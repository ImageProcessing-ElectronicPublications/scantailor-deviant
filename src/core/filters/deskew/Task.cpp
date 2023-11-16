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
#include "Settings.h"
#include "Params.h"
#include "Dependencies.h"
#include "FilterData.h"
#include "TaskStatus.h"
#include "DebugImages.h"
#include "filters/select_content/Task.h"
#include "FilterUiInterface.h"
#include "ImageView.h"
#include "DewarpingView.h"
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
	    QImage const& image,
	    ImageTransformation const& xform,
        PageId const& page_id,
        Params const& page_params,
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
    ImageTransformation m_xform;
    PageId m_pageId;
    Params m_pageParams;
    bool m_batchProcessing;
};

/*========================== Task::RotationUiUpdater =======================*/

class Task::RotationUiUpdater : public FilterResult
{
public:
    RotationUiUpdater(
        IntrusivePtr<Filter> const& filter,
        QImage const& image,
        ImageTransformation const& xform,
        PageId const& page_id,
        Params const& page_params,
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
    ImageTransformation m_xform;
    PageId m_pageId;
    Params m_pageParams;
    bool m_batchProcessing;
};

/*======================== Task::PerspectiveUiUpdater ======================*/

class Task::PerspectiveUiUpdater : public FilterResult
{
public:
    PerspectiveUiUpdater(
        IntrusivePtr<Filter> const& filter,
        QImage const& image,
        ImageTransformation const& xform,
        PageId const& page_id,
        Params const& page_params,
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
    ImageTransformation m_xform;
    PageId m_pageId;
    Params m_pageParams;
    bool m_batchProcessing;
};

/*========================= Task::DewarpingUiUpdater =======================*/

class Task::DewarpingUiUpdater : public FilterResult
{
public:
    DewarpingUiUpdater(
        IntrusivePtr<Filter> const& filter,
        QImage const& image,
        ImageTransformation const& xform,
        PageId const& page_id,
        Params const& page_params,
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
    ImageTransformation m_xform;
    PageId m_pageId;
    Params m_pageParams;
    bool m_batchProcessing;
};

/*=================================== Task =================================*/

Task::Task(
    IntrusivePtr<Filter> const& filter,
    IntrusivePtr<Settings> const& settings,
    IntrusivePtr<select_content::Task> const& next_task,
    PageId const& page_id, bool batch_processing, bool debug)
    : m_ptrFilter(filter)
    , m_ptrSettings(settings)
    , m_ptrNextTask(next_task)
    , m_pageId(page_id)
    , m_batchProcessing(batch_processing)
{
    if (debug)
    {
        m_ptrDbg.reset(new DebugImages);
    }
}

Task::~Task()
{
}

FilterResultPtr
Task::process(
    TaskStatus const& status,
    FilterData const& data)
{
    status.throwIfCancelled();

    Dependencies const deps(data.xform().preCropArea(), data.xform().preRotation());

    std::unique_ptr<Params> params(m_ptrSettings->getPageParams(m_pageId));
    std::unique_ptr<Params> old_params;

    if (params.get())
    {
        if (!deps.matches(params->dependencies()))
        {
            params.swap(old_params);
        }
    }

    if (!params.get())
    {
        params.reset(new Params(deps));
        if (old_params)
        {
            params->takeManualSettingsFrom(*old_params);
        }
    }

    switch (params->distortionType().get())
    {
    case DistortionType::NONE:
        return processNoDistortion(
                    status, data, *params
               );
    case DistortionType::ROTATION:
        return processRotationDistortion(
                    status, data, *params
               );
    case DistortionType::PERSPECTIVE:
        return processPerspectiveDistortion(
                    status, data, *params
               );
    case DistortionType::WARP:
        return processWarpDistortion(
                    status, data, *params
               );
    } // switch

    throw std::logic_error("Unexpected distortion type");
}

FilterResultPtr
Task::processNoDistortion(
    TaskStatus const& status,
    FilterData const& data,
    Params& params)
{
    // Necessary to update dependencies.
    m_ptrSettings->setPageParams(m_pageId, params);

    if (m_ptrNextTask)
    {
        return m_ptrNextTask->process(
                    status, FilterData(data, data.xform())
               );
    }
    else
    {
        return FilterResultPtr(
                   new NoDistortionUiUpdater(
                       m_ptrFilter, data.origImage(), data.xform(),
                       m_pageId, params, m_batchProcessing
                   )
               );
    }
}

FilterResultPtr
Task::processRotationDistortion(
    TaskStatus const& status,
    FilterData const& data,
    Params& params)
{
    // Necessary to update dependencies.
    m_ptrSettings->setPageParams(m_pageId, params);

    if (m_ptrNextTask)
    {
        return m_ptrNextTask->process(
                    status, FilterData(data, data.xform())
               );
    }
    else
    {
        return FilterResultPtr(
                   new RotationUiUpdater(
                       m_ptrFilter, data.origImage(), data.xform(),
                       m_pageId, params, m_batchProcessing
                   )
               );
    }
}

FilterResultPtr
Task::processPerspectiveDistortion(
    TaskStatus const& status,
    FilterData const& data,
    Params& params)
{
    // Necessary to update dependencies.
    m_ptrSettings->setPageParams(m_pageId, params);

    if (m_ptrNextTask)
    {
        return m_ptrNextTask->process(
                    status, FilterData(data, data.xform())
               );
    }
    else
    {
        return FilterResultPtr(
                   new PerspectiveUiUpdater(
                       m_ptrFilter, data.origImage(), data.xform(),
                       m_pageId, params, m_batchProcessing
                   )
               );
    }
}

FilterResultPtr
Task::processWarpDistortion(
    TaskStatus const& status,
    FilterData const& data,
    Params& params)
{
    // Necessary to update dependencies.
    m_ptrSettings->setPageParams(m_pageId, params);

    if (m_ptrNextTask)
    {
        return m_ptrNextTask->process(
                    status, FilterData(data, data.xform())
               );
    }
    else
    {
        return FilterResultPtr(
                   new DewarpingUiUpdater(
                       m_ptrFilter, data.origImage(), data.xform(),
                       m_pageId, params, m_batchProcessing
                   )
               );
    }
}

/*======================== Task::NoDistortionUiUpdater =====================*/

Task::NoDistortionUiUpdater::NoDistortionUiUpdater(
    IntrusivePtr<Filter> const& filter,
    QImage const& image,
    ImageTransformation const& xform,
    PageId const& page_id,
    Params const& page_params,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_image(image)
    , m_downscaledImage(ImageView::createDownscaledImage(image))
    , m_xform(xform)
    , m_pageId(page_id)
    , m_pageParams(page_params)
    , m_batchProcessing(batch_processing)
{
}

void
Task::NoDistortionUiUpdater::updateUI(FilterUiInterface* ui)
{
    // This function is executed from the GUI thread.

    OptionsWidget* const opt_widget = m_ptrFilter->optionsWidget();
    opt_widget->postUpdateUI(m_pageParams);
    ui->setOptionsWidget(opt_widget, ui->KEEP_OWNERSHIP);

    ui->invalidateThumbnail(m_pageId);

    if (m_batchProcessing)
    {
        return;
    }

    ImageView* view = new ImageView(m_image, m_downscaledImage, m_xform);
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP);
}

/*========================== Task::RotationUiUpdater =======================*/

Task::RotationUiUpdater::RotationUiUpdater(
    IntrusivePtr<Filter> const& filter,
    QImage const& image,
    ImageTransformation const& xform,
    PageId const& page_id,
    Params const& page_params,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_image(image)
    , m_downscaledImage(ImageView::createDownscaledImage(image))
    , m_xform(xform)
    , m_pageId(page_id)
    , m_pageParams(page_params)
    , m_batchProcessing(batch_processing)
{

}

void
Task::RotationUiUpdater::updateUI(FilterUiInterface* ui)
{
    // This function is executed from the GUI thread.

    OptionsWidget* const opt_widget = m_ptrFilter->optionsWidget();
    opt_widget->postUpdateUI(m_pageParams);
    ui->setOptionsWidget(opt_widget, ui->KEEP_OWNERSHIP);

    ui->invalidateThumbnail(m_pageId);

    if (m_batchProcessing)
    {
        return;
    }

    ImageView* view = new ImageView(m_image, m_downscaledImage, m_xform);
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP);
}

/*======================== Task::PerspectiveUiUpdater ======================*/

Task::PerspectiveUiUpdater::PerspectiveUiUpdater(
    IntrusivePtr<Filter> const& filter,
    QImage const& image,
    ImageTransformation const& xform,
    PageId const& page_id,
    Params const& page_params,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_image(image)
    , m_downscaledImage(ImageView::createDownscaledImage(image))
    , m_xform(xform)
    , m_pageId(page_id)
    , m_pageParams(page_params)
    , m_batchProcessing(batch_processing)
{

}

void
Task::PerspectiveUiUpdater::updateUI(FilterUiInterface* ui)
{
    // This function is executed from the GUI thread.

    OptionsWidget* const opt_widget = m_ptrFilter->optionsWidget();
    opt_widget->postUpdateUI(m_pageParams);
    ui->setOptionsWidget(opt_widget, ui->KEEP_OWNERSHIP);

    ui->invalidateThumbnail(m_pageId);

    if (m_batchProcessing)
    {
        return;
    }

    DewarpingView* view = new DewarpingView(m_image, m_downscaledImage, m_xform);
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP);
}

/*========================= Task::DewarpingUiUpdater =======================*/

Task::DewarpingUiUpdater::DewarpingUiUpdater(
    IntrusivePtr<Filter> const& filter,
    QImage const& image,
    ImageTransformation const& xform,
    PageId const& page_id,
    Params const& page_params,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_image(image)
    , m_downscaledImage(ImageView::createDownscaledImage(image))
    , m_xform(xform)
    , m_pageId(page_id)
    , m_pageParams(page_params)
    , m_batchProcessing(batch_processing)
{

}

void
Task::DewarpingUiUpdater::updateUI(FilterUiInterface* ui)
{
    // This function is executed from the GUI thread.

    OptionsWidget* const opt_widget = m_ptrFilter->optionsWidget();
    opt_widget->postUpdateUI(m_pageParams);
    ui->setOptionsWidget(opt_widget, ui->KEEP_OWNERSHIP);

    ui->invalidateThumbnail(m_pageId);

    if (m_batchProcessing)
    {
        return;
    }

    DewarpingView* view = new DewarpingView(m_image, m_downscaledImage, m_xform);
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP);
}

} // namespace deskew
