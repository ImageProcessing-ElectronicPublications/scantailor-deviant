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

#include "Directrix.h"
#include "PerspectiveTransform.h"
#include "HomographicTransform.h"
#include "STEX_ToPoint.h"
#include "STEX_ToVec.h"
#include <boost/array.hpp>
#include <QLineF>
#include <utility>

namespace dewarping
{

namespace Directrix
{

Plane::Plane(QPointF const& img_pt_00, QPointF const& img_pt_01,
             QPointF const& img_pt_10, QPointF const& img_pt_11,
             std::vector<QPointF> const& img_directrix)
{
    boost::array<std::pair<QPointF, QPointF>, 4> const img2pln_pairs = {
        std::make_pair(img_pt_00, QPointF(0, 0)),
        std::make_pair(img_pt_10, QPointF(1, 0)),
        std::make_pair(img_pt_01, QPointF(0, 1)),
        std::make_pair(img_pt_11, QPointF(1, 1)),
    };

    HomographicTransform<2, double> img2pln = fourPoint2DHomography(img2pln_pairs);

    m_points.reserve(img_directrix.size());

    for (QPointF const& point : img_directrix)
        m_points.push_back(toPoint(img2pln(toVec(point))));
}

Place::Place(PerspectiveTransform const& mdl2img,
             std::vector<QPointF> const& img_directrix,
             double mdl_y)
    : m_mdl2img(mdl2img)
    , m_img_directrix(img_directrix)
    , m_mdl_y(mdl_y)
    , m_img_pt_01(toPoint(mdl2img({0.0, mdl_y, 1.0})))
    , m_img_pt_11(toPoint(mdl2img({1.0, mdl_y, 1.0})))
    , m_quality(calcQuality(
        m_img_directrix.front(), m_img_pt_01, 
        m_img_directrix.back (), m_img_pt_11)
      )
{
}

double
Place::calcQuality(QPointF const& img_pt_00, QPointF const& img_pt_01,
                   QPointF const& img_pt_10, QPointF const& img_pt_11)
{
    QLineF const img_line(img_pt_00, img_pt_10);
    QLineF const img_normal(img_line.unitVector().normalVector());

    QLineF const img_line1(img_pt_00, img_pt_01);
    QLineF const img_line2(img_pt_10, img_pt_11);

    double const projection1 =
        img_line1.dx() * img_normal.dx() +
        img_line1.dy() * img_normal.dy();

    double const projection2 =
        img_line2.dx() * img_normal.dx() +
        img_line2.dy() * img_normal.dy();

    return std::abs(std::min(projection1, projection2) / img_line.length());
}

Plane
Place::createPlane() const
{
    return Plane(
        m_img_directrix.front(), m_img_pt_01,
        m_img_directrix.back (), m_img_pt_11,
        m_img_directrix
    );
}

Plane
Place::createRotatedPlane(double min_quality) const
{
    // TODO Rotate
    return Plane(
        m_img_directrix.front(), m_img_pt_01,
        m_img_directrix.back (), m_img_pt_11,
        m_img_directrix
    );
}

Profile::Profile(Plane const& plane)
{
    m_points.reserve(plane.points().size());
    
    // TODO fit to bounds
    for (QPointF const& point : plane.points())
        m_points.push_back(point);
}

} // namespace Directrix

} // namespace dewarping
