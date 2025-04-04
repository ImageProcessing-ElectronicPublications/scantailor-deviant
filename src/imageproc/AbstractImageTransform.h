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

#ifndef IMAGEPROC_ABSTRACT_IMAGE_TRANSFORM_H_
#define IMAGEPROC_ABSTRACT_IMAGE_TRANSFORM_H_

#include <QtGlobal>
#include <memory>
#include <functional>

class QColor;
class QImage;
class QSize;
class QRect;
class QPointF;
class QPolygonF;
class QTransform;
class QString;

namespace imageproc
{

class AffineImageTransform;
class AffineTransformedImage;

class AbstractImageTransform
{
    // Member-wise copying is OK.
public:
    virtual ~AbstractImageTransform() {}

    virtual std::unique_ptr<AbstractImageTransform> clone() const = 0;

    virtual bool isAffine() const = 0;

    /**
     * @brief Dimensions of original image.
     */
    virtual QSize const& origSize() const = 0;

    /**
     * @brief The crop area in original image coordinates.
     */
    virtual QPolygonF const& origCropArea() const = 0;

    /**
     * @brief The crop area in transformed coordinates.
     */
    virtual QPolygonF transformedCropArea() const = 0;

    /**
     * @brief Applies additional scaling to transformed space.
     *
     * A point (x, y) in transformed space before this call becomes point
     * (x * xscale, y * yscale) after this call. For convenience, returns
     * a QTransform that maps points in transformed space before scaling
     * to corresponding points after scaling.
     */
    virtual QTransform scale(qreal xscale, qreal yscale) = 0;

    /**
     * Similar to a full version of toAffine(), except instead of producing some
     * intermediate image plus a follow-up affine transformation, this one
     * produces an image that represents the specified area of transformed space
     * exactly, without requiring a follow-up transformation.
     */
    virtual QImage materialize(QImage const& image,
                               QRect const& target_rect, QColor const& outside_color) const = 0;

    /**
     * @brief Returns a function for mapping points from original image coordinates
     *        to transformed space.
     */
    virtual std::function<QPointF(QPointF const&)> forwardMapper() const = 0;

    /**
     * @brief Returns a function for mapping points from transformed space
     *        to original image coordinates.
     */
    virtual std::function<QPointF(QPointF const&)> backwardMapper() const = 0;
};

} // namespace imageproc

#endif
