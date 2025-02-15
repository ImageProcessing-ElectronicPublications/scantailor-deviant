/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
	Copyright (C) Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#ifndef DESKEW_DEWARPING_PARAMS_H_
#define DESKEW_DEWARPING_PARAMS_H_

#include "AutoManualMode.h"
#include "dewarping/DistortionModel.h"
#include "dewarping/FovParams.h"
#include "dewarping/FrameParams.h"
#include "dewarping/BendParams.h"
#include "dewarping/SizeParams.h"
#include "dewarping/MarginsParams.h"

class QDomDocument;
class QDomElement;
class QString;

namespace deskew
{

class DewarpingParams
{
    // Member-wise copying is OK.
public:
    /** Defaults to invalid state with MODE_AUTO. */
    DewarpingParams();

    DewarpingParams(QDomElement const& el);

    ~DewarpingParams();

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    bool isValid() const;

    void invalidate();

    AutoManualMode mode() const
    {
        return m_mode;
    }

    void setMode(AutoManualMode mode)
    {
        m_mode = mode;
    }

    dewarping::DistortionModel const& distortionModel() const
    {
        return m_distortionModel;
    }

    void setDistortionModel(dewarping::DistortionModel const& distortion_model)
    {
        m_distortionModel = distortion_model;
    }

    dewarping::FovParams& fovParams()
    {
        return m_fovParams;
    }

    dewarping::FovParams const& fovParams() const
    {
        return m_fovParams;
    }

    void setFovParams(dewarping::FovParams const& fov_params)
    {
        m_fovParams = fov_params;
    }

    dewarping::FrameParams& frameParams()
    {
        return m_frameParams;
    }

    dewarping::FrameParams const& frameParams() const
    {
        return m_frameParams;
    }

    void setFrameParams(dewarping::FrameParams const& frame_params)
    {
        m_frameParams = frame_params;
    }

    dewarping::BendParams& bendParams()
    {
        return m_bendParams;
    }

    dewarping::BendParams const& bendParams() const
    {
        return m_bendParams;
    }

    void setBendParams(dewarping::BendParams const& bend_params)
    {
        m_bendParams = bend_params;
    }

    dewarping::SizeParams& sizeParams()
    {
        return m_sizeParams;
    }

    dewarping::SizeParams const& sizeParams() const
    {
        return m_sizeParams;
    }

    void setSizeParams(dewarping::SizeParams const& size_params)
    {
        m_sizeParams = size_params;
    }

    dewarping::MarginsParams& marginsParams()
    {
        return m_marginsParams;
    }

    dewarping::MarginsParams const& marginsParams() const
    {
        return m_marginsParams;
    }

    void setMarginsParams(dewarping::MarginsParams const& margins_params)
    {
        m_marginsParams = margins_params;
    }
private:
    dewarping::DistortionModel m_distortionModel;
    dewarping::FovParams m_fovParams;
    dewarping::FrameParams m_frameParams;
    dewarping::BendParams m_bendParams;
    dewarping::SizeParams m_sizeParams;
    dewarping::MarginsParams m_marginsParams;
    AutoManualMode m_mode;
};

} // namespace deskew

#endif
