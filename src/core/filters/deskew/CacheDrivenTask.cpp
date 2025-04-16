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

#include "CacheDrivenTask.h"
#include "RotationThumbnail.h"
#include "DewarpingThumbnail.h"
#include "IncompleteThumbnail.h"
#include "ThumbnailMakerBase.h"
#include "Settings.h"
#include "PageInfo.h"
#include "ImageTransformation.h"
#include "ThumbnailBase.h"
#include "ThumbnailCollector.h"
#include "ThumbnailVersionGenerator.h"
#include "filters/select_content/CacheDrivenTask.h"
#include "dewarping/DewarpingImageTransform.h"
#include <QString>
#include <memory>

using namespace dewarping;

namespace deskew
{

CacheDrivenTask::CacheDrivenTask(
    IntrusivePtr<Settings> const& settings,
    IntrusivePtr<select_content::CacheDrivenTask> const& next_task)
    : m_ptrSettings(settings)
    , m_ptrNextTask(next_task)
{
}

CacheDrivenTask::~CacheDrivenTask()
{
}

void
CacheDrivenTask::process(
    PageInfo const& page_info, AbstractFilterDataCollector* collector,
    ImageTransformation const& xform)
{
    Dependencies const deps(xform.preCropArea(), xform.preRotation());

    std::unique_ptr<Params> params(m_ptrSettings->getPageParams(page_info.id()));
    if (!params.get() || !deps.matches(params->dependencies()) ||
        !params->validForDistortionType(params->distortionType()))
    {
        if (ThumbnailCollector* thumb_col = dynamic_cast<ThumbnailCollector*>(collector))
        {
            thumb_col->processThumbnail(
                std::unique_ptr<QGraphicsItem>(
                    new IncompleteThumbnail(
                        thumb_col->thumbnailCache(),
                        std::make_unique<ThumbnailMakerBase>(),
                        thumb_col->maxLogicalThumbSize(),
                        page_info.imageId(), QString(), xform
                    )
                )
            );
        }
        return;
    }

    if (m_ptrNextTask)
    {
        std::shared_ptr<ImageTransformation> new_transform;
        QString thumb_version;
        
        switch (params->distortionType())
        {
            case DistortionType::NONE:
            {
                auto none = std::make_shared<ImageTransformation>(xform);
                new_transform = std::move(none);
                break;
            }
            case DistortionType::ROTATION:
            {
                auto rotated = std::make_shared<ImageTransformation>(xform);
                rotated->setPostRotation(params->rotationParams().compensationAngleDeg());
                new_transform = std::move(rotated);
                break;
            }
            case DistortionType::PERSPECTIVE:
            {
                std::vector<QPointF> const top_curve
                {
                    params->perspectiveParams().corner(PerspectiveParams::TOP_LEFT),
                    params->perspectiveParams().corner(PerspectiveParams::TOP_RIGHT)
                };
                std::vector<QPointF> const bottom_curve
                {
                    params->perspectiveParams().corner(PerspectiveParams::BOTTOM_LEFT),
                    params->perspectiveParams().corner(PerspectiveParams::BOTTOM_RIGHT)
                };

                DewarpingImageTransform perspective_transform(
                    QSize(
                        xform.origRect().width(),
                        xform.origRect().height()
                    ),
                    xform.preCropArea(),
                    top_curve, bottom_curve,
                    params->perspectiveParams().fovParams(),
                    params->perspectiveParams().frameParams(),
                    BendParams(MODE_MANUAL, 0.0, 0.0, 0.0),
                    params->perspectiveParams().sizeParams()
                );

                new_transform = std::make_shared<ImageTransformation>(
                    perspective_transform.transformedCropArea().boundingRect(),
                    xform.origDpi()
                );

                if (new_transform.get())
                    new_transform->setPreCropArea(perspective_transform.transformedCropArea());

                thumb_version = ThumbnailVersionGenerator(
                    page_info.id().subPage(), DistortionType::PERSPECTIVE
                ).generate();

                break;
            }
            case DistortionType::WARP:
            {
                DewarpingImageTransform perspective_transform(
                    QSize(
                        xform.origRect().width(),
                        xform.origRect().height()
                    ),
                    xform.preCropArea(),
                    params->dewarpingParams().distortionModel().topCurve().polyline(),
                    params->dewarpingParams().distortionModel().bottomCurve().polyline(),
                    params->dewarpingParams().fovParams(),
                    params->dewarpingParams().frameParams(),
                    params->dewarpingParams().bendParams(),
                    params->dewarpingParams().sizeParams()
                );

                new_transform = std::make_shared<ImageTransformation>(
                    perspective_transform.transformedCropArea().boundingRect(),
                    xform.origDpi()
                );

                if (new_transform.get())
                    new_transform->setPreCropArea(perspective_transform.transformedCropArea());

                thumb_version = ThumbnailVersionGenerator(
                    page_info.id().subPage(), DistortionType::WARP
                ).generate();

                break;
            }
        }

        assert(new_transform);
        m_ptrNextTask->process(page_info, collector, *new_transform, thumb_version);
        return;
    }

    if (ThumbnailCollector* thumb_col = dynamic_cast<ThumbnailCollector*>(collector))
    {
        std::unique_ptr<QGraphicsItem> thumb;
        
        switch (params->distortionType())
        {
            case DistortionType::NONE:
            {
                thumb.reset(
                    new RotationThumbnail(
                        thumb_col->thumbnailCache(),
                        thumb_col->maxLogicalThumbSize(),
                        page_info.imageId(), QString(), xform,
                        0.0,
                        false
                    )
                );
                break;
            }
            case DistortionType::ROTATION:
            {
                thumb.reset(
                    new RotationThumbnail(
                        thumb_col->thumbnailCache(),
                        thumb_col->maxLogicalThumbSize(),
                        page_info.imageId(), QString(), xform,
                        params->rotationParams().compensationAngleDeg(),
                        true
                    )
                );
                break;
            }
            case DistortionType::PERSPECTIVE:
            {
                std::vector<QPointF> const top_curve
                {
                    params->perspectiveParams().corner(PerspectiveParams::TOP_LEFT),
                    params->perspectiveParams().corner(PerspectiveParams::TOP_RIGHT)
                };
                std::vector<QPointF> const bottom_curve
                {
                    params->perspectiveParams().corner(PerspectiveParams::BOTTOM_LEFT),
                    params->perspectiveParams().corner(PerspectiveParams::BOTTOM_RIGHT)
                };

                thumb.reset(
                    new DewarpingThumbnail(
                        thumb_col->thumbnailCache(),
                        thumb_col->maxLogicalThumbSize(),
                        page_info.imageId(), QString(),
                        xform, top_curve, bottom_curve,
                        params->perspectiveParams().fovParams(),
                        params->perspectiveParams().frameParams(),
                        dewarping::BendParams(MODE_MANUAL, 0.0, 0.0, 0.0)
                    )
                );
                break;
            }
            case DistortionType::WARP:
            {
                thumb.reset(
                    new DewarpingThumbnail(
                        thumb_col->thumbnailCache(),
                        thumb_col->maxLogicalThumbSize(),
                        page_info.imageId(), QString(), xform,
                        params->dewarpingParams().distortionModel().topCurve().polyline(),
                        params->dewarpingParams().distortionModel().bottomCurve().polyline(),
                        params->dewarpingParams().fovParams(),
                        params->dewarpingParams().frameParams(),
                        params->dewarpingParams().bendParams()
                    )
                );
                break;
            }
        }
        assert(thumb.get());
        thumb_col->processThumbnail(std::move(thumb));
    }
}

} // namespace deskew
