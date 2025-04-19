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
#include <utility>

namespace dewarping
{

namespace Directrix
{

Plane::Plane(QPointF const& img_pt_00, QPointF const& img_pt_01,
             QPointF const& img_pt_10, QPointF const& img_pt_11,
             std::vector<QPointF> const& img_directrix,
             double height)
{
    boost::array<std::pair<QPointF, QPointF>, 4> const img2pln_pairs = {
        std::make_pair(img_pt_00, QPointF(0, 0)),
        std::make_pair(img_pt_10, QPointF(1, 0)),
        std::make_pair(img_pt_01, QPointF(0, height)),
        std::make_pair(img_pt_11, QPointF(1, height)),
    };

    HomographicTransform<2, double> img2pln = fourPoint2DHomography(img2pln_pairs);

    m_points.reserve(img_directrix.size());

    for (QPointF const& point : img_directrix)
        m_points.push_back(toPoint(img2pln(toVec(point))));
}

Place::Place(PerspectiveTransform const& mdl2img,
             std::vector<QPointF> const& img_directrix,
             double mdl_y, double height)
    : m_mdl2img(mdl2img)
    , m_img_directrix(img_directrix)
    , m_mdl_y(mdl_y)
    , m_height(height)
    , m_img_pt_01(toPoint(mdl2img({0.0, mdl_y, height})))
    , m_img_pt_11(toPoint(mdl2img({1.0, mdl_y, height})))
    , m_img_line(
        m_img_directrix.front(),
        m_img_directrix.back()
      )
    , m_img_normal(m_img_line.unitVector().normalVector())
    , m_img_line_length(m_img_line.length())
    , m_quality(calcQuality(
        m_img_line.p1(), m_img_pt_01,
        m_img_line.p2(), m_img_pt_11)
      )
{
}

double
Place::calcQuality(QPointF const& img_pt_00, QPointF const& img_pt_01,
                   QPointF const& img_pt_10, QPointF const& img_pt_11)
{
    QLineF const img_line1(img_pt_00, img_pt_01);
    QLineF const img_line2(img_pt_10, img_pt_11);

    double const projection1 = std::abs(
        img_line1.dx() * m_img_normal.dx() +
        img_line1.dy() * m_img_normal.dy()
    );

    double const projection2 = std::abs(
        img_line2.dx() * m_img_normal.dx() +
        img_line2.dy() * m_img_normal.dy()
    );

    return std::min(projection1, projection2) / m_img_line_length;
}

Plane
Place::createPlane() const
{
    return Plane(
        m_img_line.p1(), m_img_pt_01,
        m_img_line.p2(), m_img_pt_11,
        m_img_directrix, m_height
    );
}

Plane
Place::createRotatedPlane(double min_quality) const
{
    double const img_offset = min_quality * m_img_line_length;

    QPointF const img_offset_pt_1 = m_img_normal.pointAt(+img_offset);
    QPointF const img_offset_pt_2 = m_img_normal.pointAt(-img_offset);
    
    QLineF const img_offset_line_1 = m_img_line.translated(img_offset_pt_1 - m_img_line.p1());
    QLineF const img_offset_line_2 = m_img_line.translated(img_offset_pt_2 - m_img_line.p1());

    Eigen::Vector3d const mdl_bound_pt_01 = { 0.0, m_mdl_y + 1.0, m_height };
    Eigen::Vector3d const mdl_bound_pt_11 = { 1.0, m_mdl_y + 1.0, m_height };

    QPointF const img_bound_pt_01 = toPoint(m_mdl2img(mdl_bound_pt_01));
    QPointF const img_bound_pt_11 = toPoint(m_mdl2img(mdl_bound_pt_11));

    QLineF const img_bound_line_1(m_img_pt_01, img_bound_pt_01);
    QLineF const img_bound_line_2(m_img_pt_11, img_bound_pt_11);

    QPointF img_offset_pt1x1; img_offset_line_1.intersect(img_bound_line_1, &img_offset_pt1x1);
    QPointF img_offset_pt1x2; img_offset_line_1.intersect(img_bound_line_2, &img_offset_pt1x2);
    QPointF img_offset_pt2x1; img_offset_line_2.intersect(img_bound_line_1, &img_offset_pt2x1);
    QPointF img_offset_pt2x2; img_offset_line_2.intersect(img_bound_line_2, &img_offset_pt2x2);

    boost::array<std::pair<QPointF, QPointF>, 4> const img2mdl_pairs = {
        std::make_pair(m_img_pt_01, QPointF(0, 0)),
        std::make_pair(m_img_pt_11, QPointF(1, 0)),
        std::make_pair(img_bound_pt_01, QPointF(0, m_height)),
        std::make_pair(img_bound_pt_11, QPointF(1, m_height)),
    };

    HomographicTransform<2, double> img2mdl = fourPoint2DHomography(img2mdl_pairs);

    QPointF const mdl_offset_pt1x1 = toPoint(img2mdl(toVec(img_offset_pt1x1)));
    QPointF const mdl_offset_pt1x2 = toPoint(img2mdl(toVec(img_offset_pt1x2)));
    QPointF const mdl_offset_pt2x1 = toPoint(img2mdl(toVec(img_offset_pt2x1)));
    QPointF const mdl_offset_pt2x2 = toPoint(img2mdl(toVec(img_offset_pt2x2)));

    double const mdl_offset_1 = std::min(
        mdl_offset_pt1x1.y(),
        mdl_offset_pt1x2.y()
    );

    double const mdl_offset_2 = std::max(
        mdl_offset_pt2x1.y(),
        mdl_offset_pt2x2.y()
    );

    double const mdl_offset =
        std::abs(mdl_offset_1) < std::abs(mdl_offset_2) ?
        mdl_offset_1 : mdl_offset_2;

    Eigen::Vector3d const mdl_plane_pt_01 = { 0.0, m_mdl_y + mdl_offset, m_height };
    Eigen::Vector3d const mdl_plane_pt_11 = { 1.0, m_mdl_y + mdl_offset, m_height };

    QPointF const img_plane_pt_01 = toPoint(m_mdl2img(mdl_plane_pt_01));
    QPointF const img_plane_pt_11 = toPoint(m_mdl2img(mdl_plane_pt_11));

    return Plane(
        m_img_line.p1(), img_plane_pt_01,
        m_img_line.p2(), img_plane_pt_11,
        m_img_directrix, m_height
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
