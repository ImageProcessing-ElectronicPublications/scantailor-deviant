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

#include "HomographicTransform.h"
#include <Eigen/QR>
#include <QPointF>


HomographicTransform<2, double> fourPoint2DHomography(
    boost::array<std::pair<QPointF, QPointF>, 4> const& pairs)
{
    Eigen::Matrix<double, 8, 8> A;
    Eigen::Matrix<double, 8, 1> b;
    int i = 0;

    typedef std::pair<QPointF, QPointF> Pair;
    for (Pair const& pair : pairs)
    {
        QPointF const from(pair.first);
        QPointF const to(pair.second);

        A.row(i) << -from.x(), -from.y(), -1, 0, 0, 0, to.x()* from.x(), to.x()* from.y();
        b[i] = -to.x();
        ++i;

        A.row(i) << 0, 0, 0, -from.x(), -from.y(), -1, to.y()* from.x(), to.y()* from.y();
        b[i] = -to.y();
        ++i;
    }

    auto qr = A.colPivHouseholderQr();
    if (!qr.isInvertible())
    {
        throw std::runtime_error("Failed to build 2D homography");
    }

    Eigen::Matrix<double, 8, 1> const h(qr.solve(b));
    Eigen::Matrix3d H;
    H << h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7], 1.0;

    return HomographicTransform<2, double>(H);
}

HomographicTransform<1, double> threePoint1DHomography(
    boost::array<std::pair<double, double>, 3> const& pairs)
{
    Eigen::Matrix<double, 3, 3> A;
    Eigen::Matrix<double, 3, 1> b;
    int i = 0;

    typedef std::pair<double, double> Pair;
    for (Pair const& pair : pairs)
    {
        double const from = pair.first;
        double const to = pair.second;

        A.row(i) << -from, -1, from* to;
        b[i] = -to;
        ++i;
    }

    auto qr = A.colPivHouseholderQr();
    if (!qr.isInvertible())
    {
        throw std::runtime_error("Failed to build 2D homography");
    }

    Eigen::Matrix<double, 3, 1> const h(qr.solve(b));
    Eigen::Matrix2d H;
    H << h[0], h[1], h[2], 1.0;

    return HomographicTransform<1, double>(H);
}
