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
#include "NoDistortionView.h"
#include "ImageView.h"
#include "DewarpingView.h"
#include "ImageTransformation.h"
#include "imageproc/BinaryImage.h"
#include "imageproc/BinaryThreshold.h"
#include "imageproc/SkewFinder.h"
#include "imageproc/RasterOp.h"
#include "imageproc/ReduceThreshold.h"
#include "imageproc/UpscaleIntegerTimes.h"
#include "imageproc/SeedFill.h"
#include "imageproc/Morphology.h"
#include <memory>

namespace deskew
{

using namespace imageproc;

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
    if (!params.rotationParams().isValid())
    {
        QRect const transformed_crop_rect(
            data.xform().resultingPostCropArea().boundingRect().toRect()
        );

        status.throwIfCancelled();

        if (transformed_crop_rect.isValid())
        {
            BinaryImage bw_image(
                data.grayImage(),
                transformed_crop_rect,
                data.bwThreshold()
            );

            if (m_ptrDbg.get())
            {
                m_ptrDbg->add(bw_image, "bw_image");
            }

            cleanup(status, bw_image);
            if (m_ptrDbg.get())
            {
                m_ptrDbg->add(bw_image, "after_cleanup");
            }

            status.throwIfCancelled();

            SkewFinder skew_finder;
            Skew const skew(skew_finder.findSkew(bw_image));

            if (skew.confidence() >= skew.GOOD_CONFIDENCE)
            {
                params.rotationParams().setCompensationAngleDeg(-skew.angle());
            }
            else
            {
                params.rotationParams().setCompensationAngleDeg(0);
            }
            params.rotationParams().setMode(MODE_AUTO);

            m_ptrSettings->setPageParams(m_pageId, params);

            status.throwIfCancelled();
        }
    }

    if (m_ptrNextTask)
    {
        double const angle = params.rotationParams().compensationAngleDeg();
        ImageTransformation rotated_xform(data.xform());
        rotated_xform.setPostRotation(angle);

        return m_ptrNextTask->process(
            status, FilterData(data, rotated_xform)
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

void
Task::cleanup(TaskStatus const& status, BinaryImage& image)
{
    // We don't have to clean up every piece of garbage.
    // The only concern are the horizontal shadows, which we remove here.

    BinaryImage reduced_image;

    {
        ReduceThreshold reductor(image);
        while (reductor.image().width() >= 2000 && reductor.image().height() >= 2000)
        {
            reductor.reduce(2);
        }
        reduced_image = reductor.image();
    }

    status.throwIfCancelled();

    QSize const brick(200, 14);
    BinaryImage opened(openBrick(reduced_image, brick, BLACK));
    reduced_image.release();

    status.throwIfCancelled();

    BinaryImage seed(upscaleIntegerTimes(opened, image.size(), WHITE));
    opened.release();

    status.throwIfCancelled();

    BinaryImage garbage(seedFill(seed, image, CONN8));
    seed.release();

    status.throwIfCancelled();

    rasterOp<RopSubtract<RopDst, RopSrc> >(image, garbage);
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

    NoDistortionView* view = new NoDistortionView(m_image, m_downscaledImage, m_xform);
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

    double const angle = m_pageParams.rotationParams().compensationAngleDeg();
    ImageView* view = new ImageView(m_image, m_downscaledImage, m_xform, angle);
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP);

    QObject::connect(
        view, SIGNAL(manualDeskewAngleSet(double)),
        opt_widget, SLOT(manualDeskewAngleSetExternally(double))
    );
    QObject::connect(
        opt_widget, SIGNAL(manualDeskewAngleSet(double)),
        view, SLOT(manualDeskewAngleSetExternally(double))
    );
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
