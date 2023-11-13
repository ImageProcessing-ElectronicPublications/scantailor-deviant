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
#include "PageInfo.h"
#include "ImageTransformation.h"
#include "ThumbnailBase.h"
#include "ThumbnailCollector.h"
#include "filters/select_content/CacheDrivenTask.h"

namespace deskew
{

CacheDrivenTask::CacheDrivenTask(
    IntrusivePtr<select_content::CacheDrivenTask> const& next_task)
    : m_ptrNextTask(next_task)
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
    ImageTransformation new_xform(xform);

    if (m_ptrNextTask) {
        m_ptrNextTask->process(page_info, collector, new_xform);
        return;
    }

    if (ThumbnailCollector* thumb_col = dynamic_cast<ThumbnailCollector*>(collector)) {
        thumb_col->processThumbnail(
            std::unique_ptr<QGraphicsItem>(
                new ThumbnailBase(
                    thumb_col->thumbnailCache(),
                    thumb_col->maxLogicalThumbSize(),
                    page_info.imageId(), xform
                )
            )
        );
    }
}

} // namespace deskew
