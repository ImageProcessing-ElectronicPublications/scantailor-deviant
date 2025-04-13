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

#include "PerspectiveTransform.h"

PerspectiveTransform::PerspectiveTransform(HMat const& hmat, HVec const& hvec)
{
    m_mat.block<3, 2>(0, 0) = hmat.block<3, 2>(0, 0);
    m_mat.block<3, 1>(0, 3) = hmat.block<3, 1>(0, 2);
    m_mat.block<3, 1>(0, 2) = hvec;
}

PerspectiveTransform::ImagePoint
PerspectiveTransform::operator()(ModelPoint const& from) const
{
    Eigen::Matrix<double, 4, 1> hsrc;
    hsrc << from, 1;

    Eigen::Matrix<double, 3, 1> const hdst(m_mat * hsrc);
    return hdst.topLeftCorner(2, 1) / hdst[2];
}

double
PerspectiveTransform::zSingular(boost::array<Eigen::Matrix<double, 2, 1>, 4> const& points) const
{
    double z_singular = std::numeric_limits<double>::max();

    for (Eigen::Matrix<double, 2, 1> const point : points)
    {
        double const z_singular_i = zSingular(point);
        if (std::abs(z_singular_i) < std::abs(z_singular))
        {
            z_singular = z_singular_i;
        }
    }

    return z_singular;
}

double
PerspectiveTransform::zSingular(Eigen::Matrix<double, 2, 1> const& point) const
{
    return -(m_mat(2, 0) * point(0, 0) + m_mat(2, 1) * point(1, 0) + m_mat(2, 3)) / m_mat(2, 2);
}
