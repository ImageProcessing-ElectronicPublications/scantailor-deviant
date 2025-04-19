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
#include "Thumbnail.h"
#include "IncompleteThumbnail.h"
#include "AbstractThumbnailMaker.h"
#include "ImageTransformation.h"
#include "Settings.h"
#include "PageInfo.h"
#include "PageId.h"
#include "AbstractFilterDataCollector.h"
#include "ThumbnailCollector.h"
#include "ContentBoxCollector.h"
#include "filters/page_layout/CacheDrivenTask.h"
#include <memory>
#include <iostream>

namespace select_content
{

CacheDrivenTask::CacheDrivenTask(
    IntrusivePtr<Settings> const& settings,
    IntrusivePtr<page_layout::CacheDrivenTask> const& next_task)
    :   m_ptrSettings(settings),
        m_ptrNextTask(next_task)
{
}

CacheDrivenTask::~CacheDrivenTask()
{
}

void
CacheDrivenTask::process(
    PageInfo const& page_info, AbstractFilterDataCollector* collector,
    ImageTransformation const& xform, QString const& thumb_version,
    std::unique_ptr<AbstractThumbnailMaker> thumb_maker)
{
    std::unique_ptr<Params> params(m_ptrSettings->getPageParams(page_info.id()));

    bool need_reprocess(!params.get());
    if (!need_reprocess) {
        Params p(*params.get());
        Params::Regenerate val = p.getForceReprocess();
        need_reprocess = val & Params::RegenerateThumbnail;
        if (need_reprocess && !m_ptrNextTask) {
            val = (Params::Regenerate)(val & ~Params::RegenerateThumbnail);
            p.setForceReprocess(val);
            m_ptrSettings->setPageParams(page_info.id(), p);
        }
    }

    Dependencies const deps(xform.resultingPreCropArea());
    if (need_reprocess || (!params->dependencies().matches(deps) && (params->mode() == MODE_AUTO || !params->isContentDetectionEnabled()))) {

        if (ThumbnailCollector* thumb_col = dynamic_cast<ThumbnailCollector*>(collector)) {
            thumb_col->processThumbnail(
                std::unique_ptr<QGraphicsItem>(
                    new IncompleteThumbnail(
                        thumb_col->thumbnailCache(),
                        std::move(thumb_maker),
                        thumb_col->maxLogicalThumbSize(),
                        page_info.imageId(), thumb_version, xform
                    )
                )
            );
        }

        return;
    }

    if (ContentBoxCollector* col = dynamic_cast<ContentBoxCollector*>(collector)) {
        col->process(xform, params->contentRect());
    }

    if (m_ptrNextTask) {
        m_ptrNextTask->process(page_info, collector, xform, params->contentRect(), thumb_version, std::move(thumb_maker));
        return;
    }

    if (ThumbnailCollector* thumb_col = dynamic_cast<ThumbnailCollector*>(collector)) {
        thumb_col->processThumbnail(
            std::unique_ptr<QGraphicsItem>(
                new Thumbnail(
                    thumb_col->thumbnailCache(),
                    std::move(thumb_maker),
                    thumb_col->maxLogicalThumbSize(),
                    page_info.imageId(), thumb_version, xform,
                    params->contentRect(),
                    params->isDeviant(m_ptrSettings->std(), m_ptrSettings->maxDeviation())
                )
            )
        );
    }
}

} // namespace select_content
