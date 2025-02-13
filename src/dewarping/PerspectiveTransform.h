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

#ifndef PERSPECTIVE_TRANSFORM_H_
#define PERSPECTIVE_TRANSFORM_H_

#include <Eigen/Core>

class PerspectiveTransform
{
public:
    typedef Eigen::Matrix<double, 3, 4> Mat;
    typedef Eigen::Matrix<double, 3, 1> ModelPoint;
    typedef Eigen::Matrix<double, 2, 1> ImagePoint;
    typedef Eigen::Matrix<double, 3, 3> HMat;
    typedef Eigen::Matrix<double, 3, 1> HVec;

    explicit PerspectiveTransform(HMat const& hmat, HVec const& hvec);

    ImagePoint operator()(ModelPoint const& from) const;

    Mat const& mat() const
    {
        return m_mat;
    }
private:
    Mat m_mat;
};

#endif
