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

#include "Settings.h"
#include "RelinkablePath.h"
#include "AbstractRelinker.h"
#include "../../Utils.h"

namespace deskew
{

Settings::Settings()
{
}

Settings::~Settings()
{
}

void
Settings::clear()
{
    QMutexLocker locker(&m_mutex);
    m_perPageParams.clear();
}

void
Settings::performRelinking(AbstractRelinker const& relinker)
{
    QMutexLocker locker(&m_mutex);
    PerPageParams new_params;

    for(PerPageParams::value_type const& kv: m_perPageParams)
    {
        RelinkablePath const old_path(kv.first.imageId().filePath(), RelinkablePath::File);
        PageId new_page_id(kv.first);
        new_page_id.imageId().setFilePath(relinker.substitutionPathFor(old_path));
        new_params.insert(PerPageParams::value_type(new_page_id, kv.second));
    }

    m_perPageParams.swap(new_params);
}

void
Settings::setPageParams(PageId const& page_id, Params const& params)
{
    QMutexLocker locker(&m_mutex);
    Utils::mapSetValue(m_perPageParams, page_id, params);
}

std::unique_ptr<Params>
Settings::getPageParams(PageId const& page_id) const
{
    QMutexLocker locker(&m_mutex);

    PerPageParams::const_iterator it(m_perPageParams.find(page_id));
    if (it != m_perPageParams.end())
    {
        return std::unique_ptr<Params>(new Params(it->second));
    }
    else
    {
        return std::unique_ptr<Params>();
    }
}

DistortionType
Settings::getDistortionType(PageId const& page_id) const
{
    QMutexLocker locker(&m_mutex);

    PerPageParams::const_iterator it(m_perPageParams.find(page_id));
    if (it != m_perPageParams.end())
    {
        return it->second.distortionType();
    }
    else
    {
        return Params::defaultDistortionType();
    }
}

void
Settings::setDistortionType(
    std::set<PageId> const& pages, DistortionType const& distortion_type)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.setDistortionType(distortion_type);
        }
        else
        {
            Params params((Dependencies()));
            params.setDistortionType(distortion_type);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setRotationMode(
    std::set<PageId> const& pages,
    AutoManualMode const& mode)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.rotationParams().setMode(mode);
        }
        else
        {
            Params params((Dependencies()));
            params.rotationParams().setMode(mode);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setPerspectiveMode(
    std::set<PageId> const& pages,
    AutoManualMode const& mode)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            if (mode == MODE_AUTO && it->second.perspectiveParams().mode() != MODE_AUTO)
            {
                it->second.perspectiveParams().setCorner(PerspectiveParams::TOP_LEFT, QPointF());
                it->second.perspectiveParams().setCorner(PerspectiveParams::TOP_RIGHT, QPointF());
                it->second.perspectiveParams().setCorner(PerspectiveParams::BOTTOM_LEFT, QPointF());
                it->second.perspectiveParams().setCorner(PerspectiveParams::BOTTOM_RIGHT, QPointF());
            }
            it->second.perspectiveParams().setMode(mode);
        }
        else
        {
            Params params((Dependencies()));
            params.perspectiveParams().setMode(mode);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setPerspectiveFovParams(
    std::set<PageId> const& pages,
    dewarping::FovParams const& fov_params)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.perspectiveParams().setFovParams(fov_params);
        }
        else
        {
            Params params((Dependencies()));
            params.perspectiveParams().setFovParams(fov_params);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setPerspectiveFrameParams(
    std::set<PageId> const& pages,
    dewarping::FrameParams const& frame_params)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.perspectiveParams().setFrameParams(frame_params);
        }
        else
        {
            Params params((Dependencies()));
            params.perspectiveParams().setFrameParams(frame_params);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setPerspectiveSizeParams(
    std::set<PageId> const& pages,
    dewarping::SizeParams const& size_params)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.perspectiveParams().setSizeParams(size_params);
        }
        else
        {
            Params params((Dependencies()));
            params.perspectiveParams().setSizeParams(size_params);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setDewarpingMode(
    std::set<PageId> const& pages,
    AutoManualMode const& mode)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            if (mode == MODE_AUTO && it->second.dewarpingParams().mode() != MODE_AUTO)
            {
                it->second.dewarpingParams().invalidate();
            }
            it->second.dewarpingParams().setMode(mode);
        }
        else
        {
            Params params((Dependencies()));
            params.dewarpingParams().setMode(mode);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setDewarpingFovParams(
    std::set<PageId> const& pages,
    dewarping::FovParams const& fov_params)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.dewarpingParams().setFovParams(fov_params);
        }
        else
        {
            Params params((Dependencies()));
            params.dewarpingParams().setFovParams(fov_params);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setDewarpingFrameParams(
    std::set<PageId> const& pages,
    dewarping::FrameParams const& frame_params)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.dewarpingParams().setFrameParams(frame_params);
        }
        else
        {
            Params params((Dependencies()));
            params.dewarpingParams().setFrameParams(frame_params);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setDewarpingBendParams(
    std::set<PageId> const& pages,
    dewarping::BendParams const& bend_params)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.dewarpingParams().setBendParams(bend_params);
        }
        else
        {
            Params params((Dependencies()));
            params.dewarpingParams().setBendParams(bend_params);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

void
Settings::setDewarpingSizeParams(
    std::set<PageId> const& pages,
    dewarping::SizeParams const& size_params)
{
    QMutexLocker const locker(&m_mutex);

    for (PageId const& page_id : pages)
    {
        PerPageParams::iterator it = m_perPageParams.find(page_id);
        if (it != m_perPageParams.end())
        {
            it->second.dewarpingParams().setSizeParams(size_params);
        }
        else
        {
            Params params((Dependencies()));
            params.dewarpingParams().setSizeParams(size_params);
            Utils::mapSetValue(m_perPageParams, page_id, params);
        }
    }
}

} // namespace deskew

