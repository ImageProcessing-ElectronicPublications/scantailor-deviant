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
#include "ThumbnailPixmapCache.h"
#include "ThumbnailMakerBase.h"
#include "ThumbnailVersionGenerator.h"
#include "dewarping/DewarpingImageTransform.h"
#include "dewarping/DistortionModelBuilder.h"
#include "dewarping/TextLineTracer.h"
#include "dewarping/TopBottomEdgeTracer.h"
#include "imageproc/AffineTransformedImage.h"
#include "ImageTransformation.h"
#include "imageproc/BinaryImage.h"
#include "imageproc/BinaryThreshold.h"
#include "imageproc/SkewFinder.h"
#include "imageproc/RasterOp.h"
#include "imageproc/ReduceThreshold.h"
#include "imageproc/UpscaleIntegerTimes.h"
#include "imageproc/SeedFill.h"
#include "imageproc/Morphology.h"
#include "math/XSpline.h"
#include <QString>
#include <memory>

namespace deskew
{

using namespace imageproc;
using namespace dewarping;

/*======================== Task::NoDistortionUiUpdater =====================*/

class Task::NoDistortionUiUpdater : public FilterResult
{
public:
	NoDistortionUiUpdater(
        IntrusivePtr<Filter> const& filter,
        std::unique_ptr<DebugImages> const& dbg_img,
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
    std::unique_ptr<DebugImages> const& m_ptrDbg;
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
        std::unique_ptr<DebugImages> const& dbg_img,
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
    std::unique_ptr<DebugImages> const& m_ptrDbg;
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
        std::unique_ptr<DebugImages> const& dbg_img,
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
    std::unique_ptr<DebugImages> const& m_ptrDbg;
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
        std::unique_ptr<DebugImages> const& dbg_img,
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
    std::unique_ptr<DebugImages> const& m_ptrDbg;
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
    IntrusivePtr<ThumbnailPixmapCache> const& thumbnail_cache,
    IntrusivePtr<select_content::Task> const& next_task,
    PageId const& page_id, bool batch_processing, bool debug)
    : m_ptrFilter(filter)
    , m_ptrSettings(settings)
    , m_ptrThumbnailCache(thumbnail_cache)
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

    params->perspectiveParams().frameParams().maybeUpdate(data.xform().origRect());
    params->dewarpingParams().frameParams().maybeUpdate(data.xform().origRect());

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
                    status, FilterData(data, data.xform()), QString()
               );
    }
    else
    {
        return FilterResultPtr(
                   new NoDistortionUiUpdater(
                       m_ptrFilter, m_ptrDbg, data.origImage(), data.xform(),
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
        QRectF const transformed_crop_rect(
            data.xform().transformBack().mapRect(data.xform().resultingRect())
        );

        status.throwIfCancelled();

        if (transformed_crop_rect.isValid())
        {
            BinaryImage bw_image(
                data.grayImage(),
                QRect(
                    0, 0,
                    transformed_crop_rect.width(),
                    transformed_crop_rect.height()
                ),
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
            status, FilterData(data, rotated_xform), QString()
        );
    }
    else
    {
        return FilterResultPtr(
            new RotationUiUpdater(
                m_ptrFilter, m_ptrDbg, data.origImage(), data.xform(),
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
    AffineImageTransform orig_image_transform(data.origImage().size());
    orig_image_transform.rotate(data.xform().preRotation().toDegrees());
    orig_image_transform.setOrigCropArea(data.xform().transformBack().map(data.xform().resultingPreCropArea()));

    if (!params.perspectiveParams().isValid())
    {
        switch (params.perspectiveParams().mode())
        {
        case MODE_AUTO:
        {
            DistortionModelBuilder model_builder(
                orig_image_transform.transform().inverted().map(QPointF(0, 1))
            );

            TextLineTracer::trace(
                AffineTransformedImage(data.grayImage(), orig_image_transform),
                model_builder, status, m_ptrDbg.get()
            );

            TopBottomEdgeTracer::trace(
                data.grayImage(), model_builder.verticalBounds(),
                model_builder, status, m_ptrDbg.get()
            );

            DistortionModel distortion_model(
                model_builder.tryBuildModel(
                    params.perspectiveParams().fovParams(),
                    params.perspectiveParams().frameParams(),
                    BendParams(MODE_MANUAL, 0.0, 0.0, 0.0),
                    m_ptrDbg.get(), &data.origImage()
                )
            );

            if (distortion_model.isValid())
            {
                params.perspectiveParams().setCorner(
                    PerspectiveParams::TOP_LEFT,
                    distortion_model.topCurve().polyline().front()
                );
                params.perspectiveParams().setCorner(
                    PerspectiveParams::TOP_RIGHT,
                    distortion_model.topCurve().polyline().back()
                );
                params.perspectiveParams().setCorner(
                    PerspectiveParams::BOTTOM_LEFT,
                    distortion_model.bottomCurve().polyline().front()
                );
                params.perspectiveParams().setCorner(
                    PerspectiveParams::BOTTOM_RIGHT,
                    distortion_model.bottomCurve().polyline().back()
                );
            }
            else
            {
                // Set up a trivial transformation.

                QTransform const to_orig(data.xform().transformBack());
                QRectF const transformed_box(data.xform().resultingPostCropArea().boundingRect());

                params.perspectiveParams().setCorner(
                    PerspectiveParams::TOP_LEFT, to_orig.map(transformed_box.topLeft())
                );
                params.perspectiveParams().setCorner(
                    PerspectiveParams::TOP_RIGHT, to_orig.map(transformed_box.topRight())
                );
                params.perspectiveParams().setCorner(
                    PerspectiveParams::BOTTOM_LEFT, to_orig.map(transformed_box.bottomLeft())
                );
                params.perspectiveParams().setCorner(
                    PerspectiveParams::BOTTOM_RIGHT, to_orig.map(transformed_box.bottomRight())
                );
            }

            break;
        }
        default:
        {
            // Set up a trivial transformation.

            QTransform const to_orig(data.xform().transformBack());
            QRectF const transformed_box(data.xform().resultingPostCropArea().boundingRect());

            params.perspectiveParams().setCorner(
                PerspectiveParams::TOP_LEFT, to_orig.map(transformed_box.topLeft())
            );
            params.perspectiveParams().setCorner(
                PerspectiveParams::TOP_RIGHT, to_orig.map(transformed_box.topRight())
            );
            params.perspectiveParams().setCorner(
                PerspectiveParams::BOTTOM_LEFT, to_orig.map(transformed_box.bottomLeft())
            );
            params.perspectiveParams().setCorner(
                PerspectiveParams::BOTTOM_RIGHT, to_orig.map(transformed_box.bottomRight())
            );

            break;
        }
        }
    } // if (!params.isValid())

    std::vector<QPointF> top_curve;
    std::vector<QPointF> bottom_curve;
    top_curve.push_back(params.perspectiveParams().corner(PerspectiveParams::TOP_LEFT));
    top_curve.push_back(params.perspectiveParams().corner(PerspectiveParams::TOP_RIGHT));
    bottom_curve.push_back(params.perspectiveParams().corner(PerspectiveParams::BOTTOM_LEFT));
    bottom_curve.push_back(params.perspectiveParams().corner(PerspectiveParams::BOTTOM_RIGHT));

    DewarpingImageTransform perspective_transform(
        orig_image_transform.origSize(),
        orig_image_transform.origCropArea(),
        top_curve, bottom_curve,
        params.perspectiveParams().fovParams(),
        params.perspectiveParams().frameParams(),
        BendParams(MODE_MANUAL, 0.0, 0.0, 0.0),
        params.perspectiveParams().sizeParams()
    );

    params.perspectiveParams().sizeParams().maybeUpdate(perspective_transform.imageSize());
    params.perspectiveParams().fovParams().maybeUpdate(perspective_transform.fov());

    m_ptrSettings->setPageParams(m_pageId, params);

    if (m_ptrNextTask)
    {
        QRectF const transformed_rectF = perspective_transform.transformedCropArea().boundingRect();
        QRect const transformed_rect(
            transformed_rectF.left(),
            transformed_rectF.top(),
            transformed_rectF.width(),
            transformed_rectF.height()
        );

        QImage transformed_image = perspective_transform.materialize(
            data.origImage(),
            transformed_rect,
            QColor(255, 255, 255, 0)
        );

        if (data.xform().preRotation().toDegrees() == 0)
        {
            transformed_image.setDotsPerMeterX(data.origImage().dotsPerMeterX());
            transformed_image.setDotsPerMeterY(data.origImage().dotsPerMeterY());
        }
        else
        {
            transformed_image.setDotsPerMeterX(data.origImage().dotsPerMeterY());
            transformed_image.setDotsPerMeterY(data.origImage().dotsPerMeterX());
        }

        QTransform const crop_area_transform(
            1.0, 0.0,
            0.0, 1.0,
            -transformed_rectF.left(),
            -transformed_rectF.top()
        );

        QPolygonF const crop_area = crop_area_transform.map(
            perspective_transform.transformedCropArea()
        );

        QString const thumb_version = ThumbnailVersionGenerator(
            m_pageId.subPage(), DistortionType::PERSPECTIVE
        ).generate();

        m_ptrThumbnailCache->recreateThumbnail(
            m_pageId.imageId(), thumb_version, transformed_image, ThumbnailMakerBase());

        return m_ptrNextTask->process(
            status, FilterData(data.origImageFilename(), transformed_image, crop_area), thumb_version
        );
    }
    else
    {
        return FilterResultPtr(
            new PerspectiveUiUpdater(
                m_ptrFilter, m_ptrDbg, data.origImage(), data.xform(),
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
    AffineImageTransform orig_image_transform(data.origImage().size());
    orig_image_transform.rotate(data.xform().preRotation().toDegrees());
    orig_image_transform.setOrigCropArea(data.xform().transformBack().map(data.xform().resultingPreCropArea()));

    if (!params.dewarpingParams().isValid())
    {
        switch (params.dewarpingParams().mode())
        {
        case MODE_AUTO:
        {
            DistortionModelBuilder model_builder(
                orig_image_transform.transform().inverted().map(QPointF(0, 1))
            );

            TextLineTracer::trace(
                AffineTransformedImage(data.grayImage(), orig_image_transform),
                model_builder, status, m_ptrDbg.get()
            );

            TopBottomEdgeTracer::trace(
                data.grayImage(), model_builder.verticalBounds(),
                model_builder, status, m_ptrDbg.get()
            );

            DistortionModel distortion_model(
                model_builder.tryBuildModel(
                    params.dewarpingParams().fovParams(),
                    params.dewarpingParams().frameParams(),
                    params.dewarpingParams().bendParams(),
                    m_ptrDbg.get(), &data.origImage())
            );

            if (!distortion_model.isValid())
            {
                // Set up a trivial transformation.

                QTransform const to_orig(data.xform().transformBack());
                QRectF const transformed_box(data.xform().resultingPostCropArea().boundingRect());

                distortion_model.setTopCurve(
                    std::vector<QPointF>
                    {
                        to_orig.map(transformed_box.topLeft()),
                        to_orig.map(transformed_box.topRight())
                    }
                );

                distortion_model.setBottomCurve(
                    std::vector<QPointF>
                    {
                        to_orig.map(transformed_box.bottomLeft()),
                        to_orig.map(transformed_box.bottomRight())
                    }
                );

                assert(distortion_model.isValid());
            }

            params.dewarpingParams().setDistortionModel(distortion_model);

            break;
        }
        default:
        {
            // Set up a trivial transformation.
            DistortionModel distortion_model;

            QTransform const to_orig(data.xform().transformBack());
            QRectF const transformed_box(data.xform().resultingPostCropArea().boundingRect());

            distortion_model.setTopCurve(
                std::vector<QPointF>
            {
                to_orig.map(transformed_box.topLeft()),
                    to_orig.map(transformed_box.topRight())
            }
            );

            distortion_model.setBottomCurve(
                std::vector<QPointF>
            {
                to_orig.map(transformed_box.bottomLeft()),
                    to_orig.map(transformed_box.bottomRight())
            }
            );

            assert(distortion_model.isValid());

            params.dewarpingParams().setDistortionModel(distortion_model);

            break;
        }
        }
    } // if (!params.isValid())

    DewarpingImageTransform dewarping_transform(
        orig_image_transform.origSize(),
        orig_image_transform.origCropArea(),
        params.dewarpingParams().distortionModel().topCurve().polyline(),
        params.dewarpingParams().distortionModel().bottomCurve().polyline(),
        params.dewarpingParams().fovParams(),
        params.dewarpingParams().frameParams(),
        params.dewarpingParams().bendParams(),
        params.dewarpingParams().sizeParams()
    );

    params.dewarpingParams().sizeParams().maybeUpdate(dewarping_transform.imageSize());
    params.dewarpingParams().fovParams().maybeUpdate(dewarping_transform.fov());
    params.dewarpingParams().bendParams().maybeUpdate(dewarping_transform.bend());

    m_ptrSettings->setPageParams(m_pageId, params);

    if (m_ptrNextTask)
    {

        QRectF const transformed_rectF = dewarping_transform.transformedCropArea().boundingRect();
        QRect const transformed_rect(
            transformed_rectF.left(),
            transformed_rectF.top(),
            transformed_rectF.width(),
            transformed_rectF.height()
        );

        QImage transformed_image = dewarping_transform.materialize(
            data.origImage(),
            transformed_rect,
            QColor(255, 255, 255, 0)
        );

        if (data.xform().preRotation().toDegrees() == 0)
        {
            transformed_image.setDotsPerMeterX(data.origImage().dotsPerMeterX());
            transformed_image.setDotsPerMeterY(data.origImage().dotsPerMeterY());
        }
        else
        {
            transformed_image.setDotsPerMeterX(data.origImage().dotsPerMeterY());
            transformed_image.setDotsPerMeterY(data.origImage().dotsPerMeterX());
        }

        QTransform const crop_area_transform(
            1.0, 0.0,
            0.0, 1.0,
            -transformed_rectF.left(),
            -transformed_rectF.top()
        );

        QPolygonF const crop_area = crop_area_transform.map(
            dewarping_transform.transformedCropArea()
        );

        QString const thumb_version = ThumbnailVersionGenerator(
            m_pageId.subPage(), DistortionType::WARP
        ).generate();

        m_ptrThumbnailCache->recreateThumbnail(
            m_pageId.imageId(), thumb_version, transformed_image, ThumbnailMakerBase());

        return m_ptrNextTask->process(
            status, FilterData(data.origImageFilename(), transformed_image, crop_area), thumb_version
        );
    }
    else
    {
        return FilterResultPtr(
            new DewarpingUiUpdater(
                m_ptrFilter, m_ptrDbg, data.origImage(), data.xform(),
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
    std::unique_ptr<DebugImages> const& dbg_img,
    QImage const& image,
    ImageTransformation const& xform,
    PageId const& page_id,
    Params const& page_params,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_ptrDbg(dbg_img)
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
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP, m_ptrDbg.get());
}

/*========================== Task::RotationUiUpdater =======================*/

Task::RotationUiUpdater::RotationUiUpdater(
    IntrusivePtr<Filter> const& filter,
    std::unique_ptr<DebugImages> const& dbg_img,
    QImage const& image,
    ImageTransformation const& xform,
    PageId const& page_id,
    Params const& page_params,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_ptrDbg(dbg_img)
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
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP, m_ptrDbg.get());

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
    std::unique_ptr<DebugImages> const& dbg_img,
    QImage const& image,
    ImageTransformation const& xform,
    PageId const& page_id,
    Params const& page_params,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_ptrDbg(dbg_img)
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

    XSpline top_curve;
    XSpline bottom_curve;
    top_curve.appendControlPoint(
        m_pageParams.perspectiveParams().corner(PerspectiveParams::TOP_LEFT), 0
    );
    top_curve.appendControlPoint(
        m_pageParams.perspectiveParams().corner(PerspectiveParams::TOP_RIGHT), 0
    );
    bottom_curve.appendControlPoint(
        m_pageParams.perspectiveParams().corner(PerspectiveParams::BOTTOM_LEFT), 0
    );
    bottom_curve.appendControlPoint(
        m_pageParams.perspectiveParams().corner(PerspectiveParams::BOTTOM_RIGHT), 0
    );
    DistortionModel distortion_model;
    distortion_model.setTopCurve(Curve(top_curve));
    distortion_model.setBottomCurve(Curve(bottom_curve));

    DewarpingView* view = new DewarpingView(
        m_image, m_downscaledImage, m_xform, distortion_model,
        m_pageParams.perspectiveParams().fovParams(),
        m_pageParams.perspectiveParams().frameParams(),
        dewarping::BendParams(MODE_MANUAL, 0.0, 0.0, 0.0),

        // Prevent the user from introducing curvature.
        /*fixed_number_of_control_points*/true
    );
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP, m_ptrDbg.get());

    QObject::connect(
        view, SIGNAL(distortionModelChanged(dewarping::DistortionModel const&)),
        opt_widget, SLOT(manualDistortionModelSetExternally(dewarping::DistortionModel const&))
    );
    QObject::connect(
        opt_widget, SIGNAL(fovParamsSetByUser(dewarping::FovParams const&)),
        view, SLOT(fovParamsChanged(dewarping::FovParams const&))
    );
    QObject::connect(
        opt_widget, SIGNAL(frameParamsSetByUser(dewarping::FrameParams const&)),
        view, SLOT(frameParamsChanged(dewarping::FrameParams const&))
    );
}

/*========================= Task::DewarpingUiUpdater =======================*/

Task::DewarpingUiUpdater::DewarpingUiUpdater(
    IntrusivePtr<Filter> const& filter,
    std::unique_ptr<DebugImages> const& dbg_img,
    QImage const& image,
    ImageTransformation const& xform,
    PageId const& page_id,
    Params const& page_params,
    bool batch_processing)
    : m_ptrFilter(filter)
    , m_ptrDbg(dbg_img)
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

    DewarpingView* view = new DewarpingView(
        m_image, m_downscaledImage, m_xform,
        m_pageParams.dewarpingParams().distortionModel(),
        m_pageParams.dewarpingParams().fovParams(),
        m_pageParams.dewarpingParams().frameParams(),
        m_pageParams.dewarpingParams().bendParams(),
        /*fixed_number_of_control_points*/false
    );
    ui->setImageWidget(view, ui->TRANSFER_OWNERSHIP, m_ptrDbg.get());

    QObject::connect(
        view, SIGNAL(distortionModelChanged(dewarping::DistortionModel const&)),
        opt_widget, SLOT(manualDistortionModelSetExternally(dewarping::DistortionModel const&))
    );
    QObject::connect(
        opt_widget, SIGNAL(fovParamsSetByUser(dewarping::FovParams const&)),
        view, SLOT(fovParamsChanged(dewarping::FovParams const&))
    );
    QObject::connect(
        opt_widget, SIGNAL(frameParamsSetByUser(dewarping::FrameParams const&)),
        view, SLOT(frameParamsChanged(dewarping::FrameParams const&))
    );
    QObject::connect(
        opt_widget, SIGNAL(bendParamsSetByUser(dewarping::BendParams const&)),
        view, SLOT(bendParamsChanged(dewarping::BendParams const&))
    );
}

} // namespace deskew
