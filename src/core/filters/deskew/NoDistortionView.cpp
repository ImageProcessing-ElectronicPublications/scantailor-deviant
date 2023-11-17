/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2009  Joseph Artsimovich <joseph_a@mail.ru>

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

#include "NoDistortionView.h"
#include "ImageTransformation.h"
#include "ImagePresentation.h"

namespace deskew
{

NoDistortionView::NoDistortionView(
    QImage const& image, QImage const& downscaled_image,
    ImageTransformation const& xform)
    : ImageViewBase(
        image, downscaled_image,
        ImagePresentation(xform.transform(), xform.resultingPreCropArea()))
    , m_dragHandler(*this)
    , m_zoomHandler(*this)
{
    rootInteractionHandler().makeLastFollower(m_dragHandler);
    rootInteractionHandler().makeLastFollower(m_zoomHandler);
}

NoDistortionView::~NoDistortionView()
{
}

} // namespace deskew
