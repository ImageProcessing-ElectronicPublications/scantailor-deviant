/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2008  Joseph Artsimovich <joseph_a@mail.ru>

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

#ifndef IMAGEPROC_BINARIZE_H_
#define IMAGEPROC_BINARIZE_H_

#include <QSize>

class QImage;

namespace imageproc
{

class BinaryImage;
class GrayImage;

/**
 * \brief Image binarization using Otsu's global thresholding method.
 *
 * N. Otsu (1979). "A threshold selection method from gray-level histograms".
 * http://en.wikipedia.org/wiki/Otsu%27s_method
 */
BinaryImage binarizeOtsu(QImage const& src, int delta = 0);

/**
 * \brief Image binarization using Mokji's global thresholding method.
 *
 * M. M. Mokji, S. A. R. Abu-Bakar: Adaptive Thresholding Based on
 * Co-occurrence Matrix Edge Information. Asia International Conference on
 * Modelling and Simulation 2007: 444-450
 * http://www.academypublisher.com/jcp/vol02/no08/jcp02084452.pdf
 *
 * \param src The source image.  May be in any format.
 * \param max_edge_width The maximum gradient length to consider.
 * \param min_edge_magnitude The minimum color difference in a gradient.
 * \return A black and white image.
 */
BinaryImage binarizeMokji(
    QImage const& src, unsigned max_edge_width = 3,
    unsigned min_edge_magnitude = 20);

/**
  * \brief Image binarization using Niblack's local thresholding method.
  *
  * Niblack, Wayne. An introduction to digital image processing.
  * Englewood Cliffs, N. J., Prentice Hall (1986) 115-116
  */
GrayImage binarizeNiblackMap(
    GrayImage const& src, QSize window_size, double k = 0.20);
BinaryImage binarizeNiblack(
    QImage const& src, QSize window_size,
    double k = 0.20, int delta = 0);

/**
 * \brief Image binarization using Gatos' local thresholding method.
 *
 * This implementation doesn't include the post-processing steps from
 * the above paper.
 *
 * Gatos, Basilios, Ioannis Pratikakis, and Stavros J. Perantonis.
 * "An adaptive binarization technique for low quality historical documents."
 * Document Analysis Systems VI. Springer Berlin Heidelberg, 2004. 102-113.
 */
BinaryImage binarizeGatosCleaner(
    GrayImage& wiener, BinaryImage const& niblack,
    QSize const window_size, double scale);
BinaryImage binarizeGatos(
    QImage const& src, QSize window_size, double scale = 0.6,
    double noise_sigma = 3.0, double k = 0.2, int delta = 0);

/**
 * \brief Image binarization using Sauvola's local thresholding method.
 *
 * Sauvola, J. and M. Pietikainen. 2000. "Adaptive document image binarization".
 * http://www.mediateam.oulu.fi/publications/pdf/24.pdf
 */
GrayImage binarizeSauvolaMap(
    GrayImage const& src, QSize const window_size, double k = 0.34);
BinaryImage binarizeSauvola(
    QImage const& src, QSize window_size,
    double k = 0.34, int delta = 0);

/**
 * \brief Image binarization using Wolf's local thresholding method.
 *
 * C. Wolf, J.M. Jolion, F. Chassaing. "Text localization, enhancement and
 * binarization in multimedia documents."
 * http://liris.cnrs.fr/christian.wolf/papers/icpr2002v.pdf
 *
 * \param src The image to binarize.
 * \param window_size The dimensions of a pixel neighborhood to consider.
 * \param lower_bound The minimum possible gray level that can be made white.
 * \param upper_bound The maximum possible gray level that can be made black.
 */
GrayImage binarizeWolfMap(
    GrayImage const& src, QSize const window_size, double k = 0.30);
BinaryImage binarizeWolf(
    QImage const& src, QSize window_size,
    unsigned char lower_bound = 1, unsigned char upper_bound = 254,
    double k = 0.30, int delta = 0);

BinaryImage peakThreshold(QImage const& image);

} // namespace imageproc

#endif
