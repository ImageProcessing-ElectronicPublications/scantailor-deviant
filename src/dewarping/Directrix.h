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

#ifndef DEWARPING_DIRECTRIX_H_
#define DEWARPING_DIRECTRIX_H_

#include <QPointF>
#include <vector>

class PerspectiveTransform;

namespace dewarping
{

namespace Directrix
{

class Plane
{
public:
    Plane(QPointF const& img_pt_00, QPointF const& img_pt_01,
          QPointF const& img_pt_10, QPointF const& img_pt_11,
          std::vector<QPointF> const& img_directrix);

    std::vector<QPointF> const& points() const
    {
        return m_points;
    }
private:
    std::vector<QPointF> m_points;
};

class Place
{
public:
    Place(PerspectiveTransform const& mdl2img,
        std::vector<QPointF> const& img_directrix,
        double mdl_y);

    double quality() const
    {
        return m_quality;
    }

    Plane createPlane() const;

    Plane createRotatedPlane(double min_quality) const;

private:
    double calcQuality(QPointF const& img_pt_00, QPointF const& img_pt_01,
                       QPointF const& img_pt_10, QPointF const& img_pt_11);

    PerspectiveTransform const& m_mdl2img;
    std::vector<QPointF> const& m_img_directrix;
    double m_mdl_y;
    QPointF m_img_pt_01;
    QPointF m_img_pt_11;
    double m_quality;
};

class Profile
{
public:
    Profile(Plane const& plane);

    std::vector<QPointF> const& points() const
    {
        return m_points;
    }
private:
    std::vector<QPointF> m_points;
};

} // namespace Directrix

} // namespace dewarping

#endif
