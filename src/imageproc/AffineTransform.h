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

#ifndef IMAGEPROC_TRANSFORM_H_
#define IMAGEPROC_TRANSFORM_H_

#include <QSizeF>
#include <QColor>
#include <stdint.h>

class QImage;
class QRect;
class QTransform;

namespace imageproc
{

class GrayImage;

class OutsidePixels
{
    // Member-wise copying is OK.
public:
    enum Flags
    {
        COLOR   = 1 << 0,
        NEAREST = 1 << 1,
        WEAK    = 1 << 2
    };

    /**
     * \brief Outside pixels are assumed to be of particular color.
     *
     * Outside pixels may be blended with inside pixels near the edges.
     */
    static OutsidePixels assumeColor(QColor const& color)
    {
        return OutsidePixels(COLOR, color.rgba());
    }

    /**
     * \brief Outside pixels are assumed to be of particular color.
     *
     * Outside pixels won't participate in blending operations.
     */
    static OutsidePixels assumeWeakColor(QColor const& color)
    {
        return OutsidePixels(WEAK|COLOR, color.rgba());
    }

    /**
     * \brief An outside pixel is assumed to be the same as the nearest inside pixel.
     *
     * Outside pixels won't participate in blending operations.
     */
    static OutsidePixels assumeWeakNearest()
    {
        return OutsidePixels(WEAK|NEAREST, 0xff000000);
    }

    int flags() const
    {
        return m_flags;
    }

    QRgb rgba() const
    {
        return m_rgba;
    }

    QRgb rgb() const
    {
        return m_rgba | 0xff000000;
    }

    uint8_t grayLevel() const
    {
        return static_cast<uint8_t>(qGray(m_rgba));
    }
private:
    OutsidePixels(int flags, QRgb rgba) : m_flags(flags), m_rgba(rgba) {}

    int m_flags;
    QRgb m_rgba;
};

/**
 * \brief Apply an affine transformation to the image.
 *
 * \param src The source image.
 * \param xform The transformation from source to destination.
 *        Only affine transformations are supported.
 * \param dst_rect The area in source image coordinates to return
 *        as a destination image.
 * \param outside_pixels Configures handling of pixels outside of the source image.
 * \param min_mapping_area Defines the minimum rectangle in the source image
 *        that maps to a destination pixel.  This can be used to control
 *        smoothing.
 * \return The transformed image.  It's format may differ from the
 *         source image format, for example Format_Indexed8 may
 *         be transformed to Format_RGB32, if the source image
 *         contains colors other than shades of gray.
 */
QImage affineTransform(
    QImage const& src, QTransform const& xform,
    QRect const& dst_rect, OutsidePixels outside_pixels,
    QSizeF const& min_mapping_area = QSizeF(0.9, 0.9));

/**
 * \brief Apply an affine transformation to the image.
 *
 * Same as transform(), except the source image image is converted
 * to grayscale before transforming it.
 */
GrayImage affineTransformToGray(
    QImage const& src, QTransform const& xform,
    QRect const& dst_rect, OutsidePixels outside_pixels,
    QSizeF const& min_mapping_area = QSizeF(0.9, 0.9));

} // namespace imageproc

#endif
