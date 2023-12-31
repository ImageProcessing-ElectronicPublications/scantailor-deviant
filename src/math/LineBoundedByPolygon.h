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

#ifndef LINE_BOUNDED_BY_POLYGON_H_
#define LINE_BOUNDED_BY_POLYGON_H_

#include <QLineF>
#include <QPolygonF>

/**
 * If \p line (not line segment!) intersects with \p poly,
 * writes the two most distant intersection points as the new
 * \p line endpoints and returns true. Otherwise returns false
 * and leaves \p line unmodified.
 */
bool lineBoundedByPolygon(QLineF& line, QPolygonF const& poly);

#endif
