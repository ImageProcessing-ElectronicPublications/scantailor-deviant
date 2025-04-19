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

#ifndef DESKEW_DEWARPING_THUMBNAIL_H_
#define DESKEW_DEWARPING_THUMBNAIL_H_

#include "ThumbnailBase.h"
#include "IntrusivePtr.h"
#include "dewarping/FovParams.h"
#include "dewarping/FrameParams.h"
#include "dewarping/BendParams.h"
#include <vector>

class QString;

namespace deskew
{

class DewarpingThumbnail : public ThumbnailBase
{
public:
    DewarpingThumbnail(
        IntrusivePtr<ThumbnailPixmapCache> const& thumbnail_cache,
        QSizeF const& max_size, ImageId const& image_id,
        QString const& version, ImageTransformation const& xform,
        std::vector<QPointF> const& top_curve,
        std::vector<QPointF> const& bottom_curve,
        dewarping::FovParams const& fov_params,
        dewarping::FrameParams const& frame_params,
        dewarping::BendParams const& bend_params);

    virtual void paintOverImage(
        QPainter& painter,
        QTransform const& image_to_display,
        QTransform const& thumb_to_display);
private:
    std::vector<QPointF> m_topCurve;
    std::vector<QPointF> m_bottomCurve;
    dewarping::FovParams m_fovParams;
    dewarping::FrameParams m_frameParams;
    dewarping::BendParams m_bendParams;
    bool m_isValidModel;
};

} // namespace deskew

#endif
