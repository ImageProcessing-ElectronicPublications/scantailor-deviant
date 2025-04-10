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

#include "Binarize.h"
#include "BinaryImage.h"
#include "BinaryThreshold.h"
#include "Grayscale.h"
#include "GrayImage.h"
#include "IntegralImage.h"
#include "ColorFilter.h"
#include <QImage>
#include <QRect>
#include <QDebug>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

namespace imageproc
{

BinaryImage binarizeOtsu(QImage const& src, int const delta)
{
    return BinaryImage(src, BinaryThreshold(BinaryThreshold::otsuThreshold(src) + delta));
}

BinaryImage binarizeMokji(
    QImage const& src, unsigned const max_edge_width,
    unsigned const min_edge_magnitude)
{
    BinaryThreshold const threshold(
        BinaryThreshold::mokjiThreshold(
            src, max_edge_width, min_edge_magnitude
        )
    );
    return BinaryImage(src, threshold);
}

static inline bool binaryGetBW(uint32_t const* bw_line, unsigned int x)
{
    static uint32_t const msb = uint32_t(1) << 31;
    uint32_t const mask = msb >> (x & 31);

    return (bw_line[x >> 5] & mask);
}

static inline void binarySetBW(uint32_t* bw_line, unsigned int x, bool black)
{
    static uint32_t const msb = uint32_t(1) << 31;
    uint32_t const mask = msb >> (x & 31);
    if (black)
    {
        // black
        bw_line[x >> 5] |= mask;
    }
    else
    {
        // white
        bw_line[x >> 5] &= ~mask;
    }
}

BinaryImage binarizeFromMap(GrayImage const& src, GrayImage const& threshold,
    unsigned char const lower_bound, unsigned char const upper_bound, int const delta)
{
    if (src.isNull() || threshold.isNull())
    {
        return BinaryImage();
    }

    unsigned int const w = src.width();
    unsigned int const h = src.height();
    unsigned int const wt = threshold.width();
    unsigned int const ht = threshold.height();

    if ((w != wt) || (h != ht))
    {
        return BinaryImage();
    }

    uint8_t const* src_line = src.data();
    unsigned int const src_stride = src.stride();
    uint8_t const* threshold_line = threshold.data();
    unsigned int const threshold_stride = threshold.stride();

    BinaryImage bw_img(w, h);
    if (bw_img.isNull())
    {
        return BinaryImage();
    }

    uint32_t* bw_line = bw_img.data();
    unsigned int const bw_stride = bw_img.wordsPerLine();

    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
        {
            binarySetBW(bw_line, x, (src_line[x] < lower_bound || (src_line[x] <= upper_bound && ((int)src_line[x] < ((int)threshold_line[x] + delta)))));
        }
        src_line += src_stride;
        threshold_line += threshold_stride;
        bw_line += bw_stride;
    }

    return bw_img;
}

GrayImage binarizeNiblackMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeNiblackMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();

    IntegralImage<uint32_t> integral_image(w, h);
    IntegralImage<uint64_t> integral_sqimage(w, h);

    for (int y = 0; y < h; ++y)
    {
        integral_image.beginRow();
        integral_sqimage.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = src_line[x];
            integral_image.push(pixel);
            integral_sqimage.push(pixel * pixel);
        }
        src_line += src_stride;
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    src_line = src.data();
    for (int y = 0; y < h; ++y)
    {
        int const top = std::max(0, y - window_lower_half);
        int const bottom = std::min(h, y + window_upper_half); // exclusive

        for (int x = 0; x < w; ++x)
        {
            int const left = std::max(0, x - window_left_half);
            int const right = std::min(w, x + window_right_half); // exclusive
            int const area = (bottom - top) * (right - left);
            assert(area > 0); // because window_size > 0 and w > 0 and h > 0

            QRect const rect(left, top, right - left, bottom - top);
            double const window_sum = integral_image.sum(rect);
            double const window_sqsum = integral_sqimage.sum(rect);

            double const r_area = 1.0 / area;
            double const mean = window_sum * r_area;
            double const sqmean = window_sqsum * r_area;

            double const variance = sqmean - mean * mean;
            double const stddev = sqrt(fabs(variance));

            double threshold = mean - k * stddev;

            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        src_line += src_stride;
        gray_line += gray_stride;
    }

    return gray;
}

BinaryImage binarizeNiblack(
    QImage const& src, QSize const window_size,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeNiblack: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray(src);
    if (gray.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeNiblackMap(gray, window_size, k));
    BinaryImage bw_img(binarizeFromMap(gray, threshold_map, 0, 255, delta));

    return bw_img;
}

BinaryImage binarizeGatosCleaner(
    GrayImage& wiener, BinaryImage const& niblack,
    QSize const window_size)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeGatosPostfilter: invalid window_size");
    }

    if (wiener.isNull() || niblack.isNull())
    {
        return niblack;
    }

    int const w = wiener.width();
    int const h = wiener.height();
    int const wb = niblack.width();
    int const hb = niblack.height();

    if ((w != wb) || (h != hb))
    {
        return niblack;
    }

    IntegralImage<uint32_t> niblack_bg_ii(w, h);
    IntegralImage<uint32_t> wiener_bg_ii(w, h);

    uint32_t const* niblack_line = niblack.data();
    int const niblack_stride = niblack.wordsPerLine();
    uint8_t* wiener_line = wiener.data();
    int const wiener_stride = wiener.stride();

    for (int y = 0; y < h; ++y)
    {
        niblack_bg_ii.beginRow();
        wiener_bg_ii.beginRow();
        for (int x = 0; x < w; ++x)
        {
            // bg: 1, fg: 0
            uint32_t const niblack_inverted_pixel =
                (~niblack_line[x >> 5] >> (31 - (x & 31))) & uint32_t(1);
            uint32_t const wiener_pixel = wiener_line[x];
            niblack_bg_ii.push(niblack_inverted_pixel);

            // bg: wiener_pixel, fg: 0
            wiener_bg_ii.push(wiener_pixel & ~(niblack_inverted_pixel - uint32_t(1)));
        }
        wiener_line += wiener_stride;
        niblack_line += niblack_stride;
    }

    std::vector<QRect> windows;
    for (int scale = 1;; ++scale)
    {
        windows.emplace_back(0, 0, window_size.width() * scale, window_size.height() * scale);
        if (windows.back().width() > w*2 && windows.back().height() > h * 2)
        {
            // Such a window is enough to cover the whole image when centered
            // at any of its corners.
            break;
        }
    }

    // sum(background - original) for foreground pixels according to Niblack.
    uint32_t sum_diff = 0;

    // sum(background) pixels for background pixels according to Niblack.
    uint32_t sum_bg = 0;

    QRect const image_rect(wiener.rect());
    GrayImage background(wiener);
    uint8_t* background_line = background.data();
    int const background_stride = background.stride();
    niblack_line = niblack.data();
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            for (QRect window : windows)
            {
                window.moveCenter(QPoint(x, y));
                window &= image_rect;
                uint32_t const niblack_sum_bg = niblack_bg_ii.sum(window);
                if (niblack_sum_bg == 0)
                {
                    // No background pixels in this window. Try a larger one.
                    continue;
                }

                if (binaryGetBW(niblack_line, x))
                {
                    // Foreground pixel. Interpolate from background pixels in window.
                    uint32_t const wiener_sum_bg = wiener_bg_ii.sum(window);
                    uint32_t const bg = (wiener_sum_bg + (niblack_sum_bg >> 1)) / niblack_sum_bg;
                    sum_diff += bg - background_line[x];
                    background_line[x] = bg;
                }
                else
                {
                    sum_bg += background_line[x];
                }

                break;
            }
        }
        background_line += background_stride;
        niblack_line += niblack_stride;
    }

    double const delta = double(sum_diff) / (w*h - niblack_bg_ii.sum(image_rect));
    double const b = double(sum_bg) / niblack_bg_ii.sum(image_rect);

    double const q = 0.6;
    double const p1 = 0.5;
    double const p2 = 0.8;

    double const exp_scale = -4.0 / (b * (1.0 - p1));
    double const exp_bias = 2.0 * (1.0 + p1) / (1.0 - p1);
    double const threshold_scale = q * delta * (1.0 - p2);
    double const threshold_bias = q * delta * p2;

    wiener_line = wiener.data();
    background_line = background.data();
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            uint8_t& wiener_pixel = wiener_line[x];
            uint8_t const bg_pixel = background_line[x];
            double const threshold = threshold_scale /
                (1.0 + exp(double(bg_pixel) * exp_scale + exp_bias)) + threshold_bias;
            wiener_pixel = double(bg_pixel) - double(wiener_pixel) > threshold ? 0x00 : 0xff;
        }
        wiener_line += wiener_stride;
        background_line += background_stride;
    }

    return BinaryImage(wiener);
}

BinaryImage binarizeGatos(
    QImage const& src, QSize const window_size,
    double const noise_sigma, double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeGatos: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray(src);
    if (gray.isNull())
    {
        return BinaryImage();
    }

    GrayImage wiener(wienerFilter(gray, QSize(5, 5), noise_sigma));
    BinaryImage niblack(binarizeNiblack(wiener, window_size, k, delta));
    BinaryImage bw_img(binarizeGatosCleaner(wiener, niblack, window_size));

    return bw_img;
}

GrayImage binarizeSauvolaMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeSauvola: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();

    IntegralImage<uint32_t> integral_image(w, h);
    IntegralImage<uint64_t> integral_sqimage(w, h);

    for (int y = 0; y < h; ++y) {
        integral_image.beginRow();
        integral_sqimage.beginRow();
        for (int x = 0; x < w; ++x) {
            uint32_t const pixel = gray_line[x];
            integral_image.push(pixel);
            integral_sqimage.push(pixel * pixel);
        }
        gray_line += gray_stride;
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    gray_line = gray.data();
    for (int y = 0; y < h; ++y) {
        int const top = std::max(0, y - window_lower_half);
        int const bottom = std::min(h, y + window_upper_half); // exclusive

        for (int x = 0; x < w; ++x) {
            int const left = std::max(0, x - window_left_half);
            int const right = std::min(w, x + window_right_half); // exclusive
            int const area = (bottom - top) * (right - left);
            assert(area > 0); // because window_size > 0 and w > 0 and h > 0

            QRect const rect(left, top, right - left, bottom - top);
            long double const window_sum = integral_image.sum(rect);
            long double const window_sqsum = integral_sqimage.sum(rect);

            long double const r_area = 1.0 / area;
            long double const mean = window_sum * r_area;
            long double const sqmean = window_sqsum * r_area;

            long double const variance = sqmean - mean * mean;
            long double const deviation = sqrt(fabs(variance));

            long double threshold = mean * (1.0 + k * (deviation / 128.0 - 1.0));

            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
    }

    return gray;
}

BinaryImage binarizeSauvola(
    QImage const& src, QSize const window_size,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeSauvola: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage const gray(src);
    if (gray.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeSauvolaMap(gray, window_size, k));
    BinaryImage bw_img(binarizeFromMap(gray, threshold_map, 0, 255, delta));

    return bw_img;
}

GrayImage binarizeWolfMap(
    GrayImage const& src, QSize const window_size, double const k)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeWolf: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();

    IntegralImage<uint32_t> integral_image(w, h);
    IntegralImage<uint64_t> integral_sqimage(w, h);

    uint32_t min_gray_level = 255;

    for (int y = 0; y < h; ++y) {
        integral_image.beginRow();
        integral_sqimage.beginRow();
        for (int x = 0; x < w; ++x) {
            uint32_t const pixel = gray_line[x];
            integral_image.push(pixel);
            integral_sqimage.push(pixel * pixel);
            min_gray_level = std::min(min_gray_level, pixel);
        }
        gray_line += gray_stride;
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    std::vector<float> means(w * h, 0);
    std::vector<float> deviations(w * h, 0);

    long double max_deviation = 0;

    for (int y = 0; y < h; ++y) {
        int const top = std::max(0, y - window_lower_half);
        int const bottom = std::min(h, y + window_upper_half); // exclusive

        for (int x = 0; x < w; ++x) {
            int const left = std::max(0, x - window_left_half);
            int const right = std::min(w, x + window_right_half); // exclusive
            int const area = (bottom - top) * (right - left);
            assert(area > 0); // because window_size > 0 and w > 0 and h > 0

            QRect const rect(left, top, right - left, bottom - top);
            long double const window_sum = integral_image.sum(rect);
            long double const window_sqsum = integral_sqimage.sum(rect);

            long double const r_area = 1.0 / area;
            long double const mean = window_sum * r_area;
            long double const sqmean = window_sqsum * r_area;

            long double const variance = sqmean - mean * mean;
            long double const deviation = sqrt(fabs(variance));
            max_deviation = std::max(max_deviation, deviation);
            means[w * y + x] = mean;
            deviations[w * y + x] = deviation;
        }
    }

    // TODO: integral images can be disposed at this point.

    gray_line = gray.data();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float const mean = means[y * w + x];
            float const deviation = deviations[y * w + x];
            long double const a = 1.0 - deviation / max_deviation;
            long double threshold = mean - k * a * (mean - min_gray_level);

            threshold = (threshold < 0.0) ? 0.0 : ((threshold < 255.0) ? threshold : 255.0);
            gray_line[x] = (uint8_t) threshold;
        }
        gray_line += gray_stride;
    }

    return gray;
}

BinaryImage binarizeWolf(
    QImage const& src, QSize const window_size,
    unsigned char const lower_bound, unsigned char const upper_bound,
    double const k, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeWolf: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage const gray(src);
    if (gray.isNull())
    {
        return BinaryImage();
    }

    GrayImage threshold_map(binarizeWolfMap(gray, window_size, k));
    BinaryImage bw_img(binarizeFromMap(gray, threshold_map, lower_bound, upper_bound, delta));

    return bw_img;
}

BinaryImage binarizeEdgeDiv(
    QImage const& src, QSize const window_size,
    double const kep, double const kbd, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeBlurDiv: invalid windowSize");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    QImage gray(toGrayscale(src));
    if (gray.isNull())
    {
        return BinaryImage();
    }

    int const w = gray.width();
    int const h = gray.height();

    IntegralImage<uint32_t> integral_image(w, h);

    uint8_t* gray_line = gray.bits();
    int const gray_stride = gray.bytesPerLine();

    for (int y = 0; y < h; ++y)
    {
        integral_image.beginRow();
        for (int x = 0; x < w; ++x)
        {
            uint32_t const pixel = gray_line[x];
            integral_image.push(pixel);
        }
        gray_line += gray_stride;
    }

    int const window_lower_half = window_size.height() >> 1;
    int const window_upper_half = window_size.height() - window_lower_half;
    int const window_left_half = window_size.width() >> 1;
    int const window_right_half = window_size.width() - window_left_half;

    gray_line = gray.bits();
    for (int y = 0; y < h; ++y)
    {
        int const top = std::max(0, y - window_lower_half);
        int const bottom = std::min(h, y + window_upper_half);  // exclusive
        for (int x = 0; x < w; ++x)
        {
            int const left = std::max(0, x - window_left_half);
            int const right = std::min(w, x + window_right_half);  // exclusive
            int const area = (bottom - top) * (right - left);
            assert(area > 0);  // because windowSize > 0 and w > 0 and h > 0
            QRect const rect(left, top, right - left, bottom - top);
            double const window_sum = integral_image.sum(rect);

            double const r_area = 1.0 / area;
            double const mean = window_sum * r_area;
            double const origin = gray_line[x];
            double retval = origin;
            if (kep > 0.0)
            {
                // EdgePlus
                // edge = I / blur (shift = -0.5) {0.0 .. >1.0}, mean value = 0.5
                double const edge = (retval + 1) / (mean + 1) - 0.5;
                // edgeplus = I * edge, mean value = 0.5 * mean(I)
                double const edgeplus = origin * edge;
                // return k * edgeplus + (1 - k) * I
                retval = kep * edgeplus + (1.0 - kep) * origin;
            }
            if (kbd > 0.0)
            {
                // BlurDiv
                // edge = blur / I (shift = -0.5) {0.0 .. >1.0}, mean value = 0.5
                double const edgeinv = (mean + 1) / (retval + 1) - 0.5;
                // edgenorm = edge * k + max * (1 - k), mean value = {0.5 .. 1.0} * mean(I)
                double const edgenorm = kbd * edgeinv + (1.0 - kbd);
                // return I / edgenorm
                retval = (edgenorm > 0.0) ? (origin / edgenorm) : origin;
            }
            // trim value {0..255}
            retval = (retval < 0.0) ? 0.0 : (retval < 255.0) ? retval : 255.0;
            gray_line[x] = (int) retval;
        }
        gray_line += gray_stride;
    }
    return binarizeOtsu(gray, delta);
}

BinaryImage
peakThreshold(QImage const& image)
{
    return BinaryImage(image, BinaryThreshold::peakThreshold(image));
}

GrayImage binarizeMScaleMap(
    GrayImage const& src, QSize const window_size, double const coef)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeMScaleMap: invalid window_size");
    }

    if (src.isNull())
    {
        return GrayImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return GrayImage();
    }

    int const w = src.width();
    int const h = src.height();
    uint8_t const* src_line = src.data();
    int const src_stride = src.stride();
    uint8_t* gray_line = gray.data();
    int const gray_stride = gray.stride();

    unsigned int whcp, l, i, j, blsz, rsz, radius;
    double immean, kover, sensitivity, sensdiv, senspos, sensinv;
    unsigned int pim, immin, immax, imt, cnth, cntw, level = 0;
    unsigned int maskbl, maskover, tim;
    unsigned long int idx;

    radius = (window_size.height() + window_size.width()) >> 1;
    whcp = (h + w) >> 1;
    blsz = 1;
    while (blsz < whcp)
    {
        level++;
        blsz <<= 1;
    }
    blsz >>= 1;
    rsz = 1;
    while ((rsz < radius) && (level > 1))
    {
        level--;
        rsz <<= 1;
    }

    gray_line = gray.data();
    immin = gray_line[0];
    immax = immin;
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            pim = gray_line[x];
            if (pim < immin)
            {
                immin = pim;
            }
            if (pim > immax)
            {
                immax = pim;
            }
        }
        gray_line += gray_stride;
    }
    immean = (double) (immax + immin);
    immean *= 0.5;
    immean += 0.5;
    tim = (unsigned int) immean;

    gray_line = gray.data();
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            gray_line[x] = tim;
        }
        gray_line += gray_stride;
    }

    kover = 1.5;

    if (coef < 0.0)
    {
        sensitivity = -coef;
        sensdiv = sensitivity;
        sensdiv += 1.0;
        sensinv = 1.0 / sensdiv;
        senspos = sensitivity / sensdiv;
    }
    else
    {
        sensitivity = coef;
        sensdiv = sensitivity;
        sensdiv += 1.0;
        senspos = 1.0 / sensdiv;
        sensinv = sensitivity / sensdiv;
    }

    src_line = src.data();
    gray_line = gray.data();
    for (l = 0; l < level; l++)
    {
        cnth = (h + blsz - 1) / blsz;
        cntw = (w + blsz - 1) / blsz;
        maskbl = blsz;
        maskover = (unsigned int) (kover * maskbl);
        for (i = 0; i < cnth; i++)
        {
            int y0 = i * maskbl;
            int y1 = y0 + maskover;
            y1 = (y1 < h) ? y1 : h;
            for (j = 0; j < cntw; j++)
            {
                int x0 = j * maskbl;
                int x1 = x0 + maskover;
                x1 = (x1 < w) ? x1 : w;

                idx = y0 * src_stride + x0;
                immin = src_line[idx];
                immax = immin;
                for (int y = y0; y < y1; y++)
                {
                    for (int x = x0; x < x1; x++)
                    {
                        idx = y * src_stride + x;
                        pim = src_line[idx];
                        if (pim < immin)
                        {
                            immin = pim;
                        }
                        if (pim > immax)
                        {
                            immax = pim;
                        }
                    }
                }
                immean = (double) (immax + immin);
                immean *= 0.5;
                immean *= sensinv;
                for (int y = y0; y < y1; y++)
                {
                    for (int x = x0; x < x1; x++)
                    {
                        idx = y * gray_stride + x;
                        imt = gray_line[idx];
                        imt *= senspos;
                        imt += immean;
                        imt += 0.5;
                        imt = (imt < 0.0) ? 0.0 : ((imt < 255.0) ? imt : 255.0);
                        gray_line[idx] = (uint8_t) imt;
                    }
                }
            }
        }
        blsz >>= 1;
    }

    return gray;
}  // binarizeMScaleMap

BinaryImage binarizeMScale(
    QImage const& src, QSize const window_size,
    double const coef, int const delta)
{
    if (window_size.isEmpty())
    {
        throw std::invalid_argument("binarizeMScale: invalid window_size");
    }

    if (src.isNull())
    {
        return BinaryImage();
    }

    GrayImage gray = GrayImage(src);
    if (gray.isNull())
    {
        return BinaryImage();
    }


    GrayImage threshold_map(binarizeMScaleMap(gray, window_size, coef));
    BinaryImage bw_img(binarizeFromMap(gray, threshold_map, 0, 255, delta));

    return bw_img;
}  // binarizeMScale

} // namespace imageproc
