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
