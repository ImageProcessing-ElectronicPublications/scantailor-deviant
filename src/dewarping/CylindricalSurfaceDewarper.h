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

#ifndef DEWARPING_CYLINDRICAL_SURFACE_DEWARPER_H_
#define DEWARPING_CYLINDRICAL_SURFACE_DEWARPER_H_

#include "HomographicTransform.h"
#include "PerspectiveTransform.h"
#include "PolylineIntersector.h"
#include "ArcLengthMapper.h"
#include "ImageSize.h"
#include <boost/array.hpp>
#include <vector>
#include <utility>
#include <QPointF>
#include <QLineF>

namespace dewarping
{

class FovParams;
class FrameParams;
class BendParams;
class SizeParams;

/**
 * @brief A model for mapping a curved quadrilateral into a rectangle.
 *
 * Implements a model from [17] plus a homographic transform on top.
 *
 * @see Help -> About -> References -> [17]
 */
class CylindricalSurfaceDewarper
{
public:
    class State
    {
        friend class CylindricalSurfaceDewarper;
    private:
        PolylineIntersector::Hint m_intersectionHint1;
        PolylineIntersector::Hint m_intersectionHint2;
        ArcLengthMapper::Hint m_arcLengthHint;
    };

    struct Generatrix
    {
        QLineF imgLine;
        HomographicTransform<1, double> pln2img;

        Generatrix(QLineF const& img_line, HomographicTransform<1, double> const& H)
            : imgLine(img_line), pln2img(H) {}
    };

    CylindricalSurfaceDewarper(
        std::vector<QPointF> const& img_directrix1,
        std::vector<QPointF> const& img_directrix2,
        FovParams const& fov_params,
        FrameParams const& frame_params,
        BendParams const& bend_params);

    /**
     * \brief Returns the arc length of a directrix, assuming its
     *        chord length is one.
     */
    double directrixArcLength() const
    {
        return m_directrixArcLength;
    }

    double fov() const
    {
        return m_fov;
    }

    double bend() const
    {
        return m_bend;
    }

    ImageSize imageSize(
        std::vector<QPointF> const& img_directrix1,
        std::vector<QPointF> const& img_directrix2,
        SizeParams const& size_params) const;

    Generatrix mapGeneratrix(double crv_x, State& state) const;

    /**
     * Transforms a point from warped image coordinates
     * to dewarped normalized coordinates.  See comments
     * in the beginning of the *.cpp file for more information
     * about coordinate systems we work with.
     */
    QPointF mapToDewarpedSpace(QPointF const& img_pt) const;

    /**
     * This version may achieve higher performance when transforming many
     * points at once.
     */
    QPointF mapToDewarpedSpace(QPointF const& img_pt, State& state) const;

    /**
     * Transforms a point from dewarped normalized coordinates
     * to warped image coordinates.  See comments in the beginning
     * of the *.cpp file for more information about coordinate
     * systems we owork with.
     */
    QPointF mapToWarpedSpace(QPointF const& crv_pt) const;
private:
    static HomographicTransform<2, double> calcPlnToImgHomography(
        std::vector<QPointF> const& img_directrix1,
        std::vector<QPointF> const& img_directrix2);

    PerspectiveTransform calcMdlToImgTransform(
        HomographicTransform<2, double> const& pln2img,
        FovParams const& fov_params,
        FrameParams const& frame_params);

    void initArcLengthMapper(
        std::vector<QPointF> const& img_directrix1,
        std::vector<QPointF> const& img_directrix2,
        BendParams const& bend_params);

    HomographicTransform<2, double> m_pln2img;
    HomographicTransform<2, double> m_img2pln;
    PerspectiveTransform m_mdl2img;
    double m_fov;
    double m_bend;
    double m_Sx;
    double m_Sy;
    double m_directrixArcLength;
    ArcLengthMapper m_arcLengthMapper;
    PolylineIntersector m_imgDirectrix1Intersector;
    PolylineIntersector m_imgDirectrix2Intersector;
};

} // namespace dewarping

#endif
