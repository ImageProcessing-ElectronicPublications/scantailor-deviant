/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#include "CommandLine.h"
#include "OutputGenerator.h"
#include "ImageTransformation.h"
#include "FilterData.h"
#include "TaskStatus.h"
#include "Utils.h"
#include "DebugImages.h"
#include "EstimateBackground.h"
#include "Despeckle.h"
#include "RenderParams.h"
#include "Dpi.h"
#include "Dpm.h"
#include "Zone.h"
#include "ZoneSet.h"
#include "PictureLayerProperty.h"
#include "FillColorProperty.h"
#include "imageproc/GrayImage.h"
#include "imageproc/BinaryImage.h"
#include "imageproc/BinaryThreshold.h"
#include "imageproc/Binarize.h"
#include "imageproc/BWColor.h"
#include "imageproc/Transform.h"
#include "imageproc/Scale.h"
#include "imageproc/Morphology.h"
#include "imageproc/Connectivity.h"
#include "imageproc/ConnCompEraser.h"
#include "imageproc/SeedFill.h"
#include "imageproc/Constants.h"
#include "imageproc/Grayscale.h"
#include "imageproc/RasterOp.h"
#include "imageproc/GrayRasterOp.h"
#include "imageproc/PolynomialSurface.h"
#include "imageproc/SavGolFilter.h"
#include "imageproc/DrawOver.h"
#include "imageproc/AdjustBrightness.h"
#include "imageproc/PolygonRasterizer.h"
#include "imageproc/ConnectivityMap.h"
#include "imageproc/InfluenceMap.h"
#include "config.h"
#include "settings/globalstaticsettings.h"
#ifdef HAVE_EXIV2
#include "ImageMetadataCopier.h"
#endif
#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#endif
#include <QImage>
#include <QSize>
#include <QPoint>
#include <QRect>
#include <QRectF>
#include <QPointF>
#include <QPolygonF>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QtGlobal>
#include <QDebug>
#include <Qt>
#include <vector>
#include <memory>
#include <new>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include <stdint.h>
//begin of modified by monday2000
//Marginal_Dewarping
#include "imageproc/OrthogonalRotation.h"
//end of modified by monday2000

using namespace imageproc;
using namespace dewarping;

namespace output
{

namespace
{

struct RaiseAboveBackground {
    static uint8_t transform(uint8_t src, uint8_t dst)
    {
        // src: orig
        // dst: background (dst >= src)
        if (dst - src < 1) {
            return 0xff;
        }
        unsigned const orig = src;
        unsigned const background = dst;
        return static_cast<uint8_t>((orig * 255 + background / 2) / background);
    }
};

struct CombineInverted {
    static uint8_t transform(uint8_t src, uint8_t dst)
    {
        unsigned const dilated = dst;
        unsigned const eroded = src;
        unsigned const res = 255 - (255 - dilated) * eroded / 255;
        return static_cast<uint8_t>(res);
    }
};

/**
 * In picture areas we make sure we don't use pure black and pure white colors.
 * These are reserved for text areas.  This behaviour makes it possible to
 * detect those picture areas later and treat them differently, for example
 * encoding them as a background layer in DjVu format.
 */
template<typename PixelType>
PixelType reserveBlackAndWhite(PixelType color);

template<>
uint32_t reserveBlackAndWhite(uint32_t color)
{
    // We handle both RGB32 and ARGB32 here.
    switch (color & 0x00FFFFFF) {
    case 0x00000000:
        return 0xFF010101;
    case 0x00FFFFFF:
        return 0xFFFEFEFE;
    default:
        return color;
    }
}

template<>
uint8_t reserveBlackAndWhite(uint8_t color)
{
    switch (color) {
    case 0x00:
        return 0x01;
    case 0xFF:
        return 0xFE;
    default:
        return color;
    }
}

template<typename PixelType>
void reserveBlackAndWhite(QSize size, int stride, PixelType* data)
{
    int const width = size.width();
    int const height = size.height();

    PixelType* line = data;
    for (int y = 0; y < height; ++y, line += stride) {
        for (int x = 0; x < width; ++x) {
            line[x] = reserveBlackAndWhite<PixelType>(line[x]);
        }
    }
}

void reserveBlackAndWhite(QImage& img)
{
    assert(img.depth() == 8 || img.depth() == 24 || img.depth() == 32);
    switch (img.format()) {
    case QImage::Format_Indexed8:
        reserveBlackAndWhite(img.size(), img.bytesPerLine(), img.bits());
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        reserveBlackAndWhite(img.size(), img.bytesPerLine() / 4, (uint32_t*)img.bits());
        break;
    default:; // Should not happen.
    }
}

/**
 * Fills areas of \p mixed with pixels from \p bw_content in
 * areas where \p bw_mask is black.  Supported \p mixed image formats
 * are Indexed8 grayscale, RGB32 and ARGB32.
 * The \p MixedPixel type is uint8_t for Indexed8 grayscale and uint32_t
 * for RGB32 and ARGB32.
 */
template<typename MixedPixel>
void combineMixed(
    QImage& mixed, BinaryImage const& bw_content,
    BinaryImage const& bw_mask)
{
    MixedPixel* mixed_line = reinterpret_cast<MixedPixel*>(mixed.bits());
    int const mixed_stride = mixed.bytesPerLine() / sizeof(MixedPixel);
    uint32_t const* bw_content_line = bw_content.data();
    int const bw_content_stride = bw_content.wordsPerLine();
    uint32_t const* bw_mask_line = bw_mask.data();
    int const bw_mask_stride = bw_mask.wordsPerLine();
    int const width = mixed.width();
    int const height = mixed.height();
    uint32_t const msb = uint32_t(1) << 31;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (bw_mask_line[x >> 5] & (msb >> (x & 31))) {
                // B/W content.

                uint32_t tmp = bw_content_line[x >> 5];
                tmp >>= (31 - (x & 31));
                tmp &= uint32_t(1);
                // Now it's 0 for white and 1 for black.

                --tmp; // 0 becomes 0xffffffff and 1 becomes 0.

                tmp |= 0xff000000; // Force opacity.

                mixed_line[x] = static_cast<MixedPixel>(tmp);
            } else {
                // Non-B/W content.
                mixed_line[x] = reserveBlackAndWhite<MixedPixel>(mixed_line[x]);
            }
        }
        mixed_line += mixed_stride;
        bw_content_line += bw_content_stride;
        bw_mask_line += bw_mask_stride;
    }
}

} // anonymous namespace

OutputGenerator::OutputGenerator(
    Dpi const& dpi, ColorParams const& color_params,
    DespeckleLevel const despeckle_level,
    ImageTransformation const& xform,
    QPolygonF const& content_rect_phys)
    :   m_dpi(dpi),
        m_colorParams(color_params),
        m_xform(xform),
        m_outRect(xform.resultingRect().toAlignedRect()),
        m_contentRect(xform.transform().map(content_rect_phys).boundingRect().toAlignedRect()),
        m_despeckleLevel(despeckle_level)
{
    /*
    std::cout << "m_outRect.left(): " << m_outRect.left() << " right(): " << m_outRect.right() << " top: " << m_outRect.top() << " bottom: " << m_outRect.bottom() << std::endl;
    std::cout << "m_contentRect.left(): " << m_contentRect.left() << " right(): " << m_contentRect.right() << " top: " << m_contentRect.top() << " bottom: " << m_contentRect.bottom() << std::endl;
    */

    // Sometimes `toAlignedRect()` may return rect with coordinate < 0.0
    if (m_outRect.left() < 0) {
        m_outRect.setLeft(0);
    }
    if (m_outRect.top() < 0) {
        m_outRect.setTop(0);
    }
    if (m_contentRect.left() < m_outRect.left()) {
        m_contentRect.setLeft(m_outRect.left());
    }
    if (m_contentRect.top() < m_outRect.top()) {
        m_contentRect.setTop(m_outRect.top());
    }
    if (m_contentRect.right() > m_outRect.right()) {
        m_contentRect.setRight(m_outRect.right());
    }
    if (m_contentRect.bottom() > m_outRect.bottom()) {
        m_contentRect.setBottom(m_outRect.bottom());
    }

    assert(m_outRect.topLeft() == QPoint(0, 0));

    // Note that QRect::contains(<empty rect>) always returns false, so we don't use it here.
    assert(m_outRect.contains(m_contentRect.topLeft()) && m_outRect.contains(m_contentRect.bottomRight()));
}

QImage
OutputGenerator::process(
    TaskStatus const& status, FilterData const& input,
//Quadro_Zoner
    //ZoneSet const& picture_zones, ZoneSet const& fill_zones,
    ZoneSet& picture_zones, ZoneSet const& fill_zones,
//Original_Foreground_Mixed
    bool keep_orig_fore_subscan,
    imageproc::BinaryImage* auto_layer_mask,
    imageproc::BinaryImage* speckles_image,
    DebugImages* const dbg,
    PageId* p_pageId,
    IntrusivePtr<Settings>* p_settings
) const
{
    QImage image(
        processImpl(
            status, input, picture_zones, fill_zones,
            keep_orig_fore_subscan,
            auto_layer_mask, speckles_image, dbg,
            p_pageId, p_settings
        )
    );
    assert(!image.isNull());

    // Set the correct DPI.
    Dpm const output_dpm(m_dpi);
    image.setDotsPerMeterX(output_dpm.horizontal());
    image.setDotsPerMeterY(output_dpm.vertical());

    return image;
}

QSize
OutputGenerator::outputImageSize() const
{
    return m_outRect.size();
}

QRect
OutputGenerator::outputContentRect() const
{
    return m_contentRect;
}

GrayImage
OutputGenerator::normalizeIlluminationGray(
    TaskStatus const& status,
    QImage const& input, QPolygonF const& area_to_consider,
    QTransform const& xform, QRect const& target_rect,
    GrayImage* background, DebugImages* const dbg)
{
    GrayImage to_be_normalized(
        transformToGray(
            input, xform, target_rect, OutsidePixels::assumeWeakNearest()
        )
    );
    if (dbg) {
        dbg->add(to_be_normalized, "to_be_normalized");
    }

    status.throwIfCancelled();

    QPolygonF transformed_consideration_area(xform.map(area_to_consider));
    transformed_consideration_area.translate(-target_rect.topLeft());

    PolynomialSurface const bg_ps(
        estimateBackground(
            to_be_normalized, transformed_consideration_area,
            status, dbg
        )
    );

    status.throwIfCancelled();

    GrayImage bg_img(bg_ps.render(to_be_normalized.size()));
    if (dbg) {
        dbg->add(bg_img, "background");
    }
    if (background) {
        *background = bg_img;
    }

    status.throwIfCancelled();

    grayRasterOp<RaiseAboveBackground>(bg_img, to_be_normalized);
    if (dbg) {
        dbg->add(bg_img, "normalized_illumination");
    }

    return bg_img;
}

imageproc::BinaryImage
OutputGenerator::estimateBinarizationMask(
    TaskStatus const& status, GrayImage const& gray_source,
    QRect const& source_rect, QRect const& source_sub_rect,
    DebugImages* const dbg) const
{
    assert(source_rect.contains(source_sub_rect));

    // If we need to strip some of the margins from a grayscale
    // image, we may actually do it without copying anything.
    // We are going to construct a QImage from existing data.
    // That image won't own that data, but gray_source is not
    // going anywhere, so it's fine.

    GrayImage trimmed_image;

    if (source_rect == source_sub_rect) {
        trimmed_image = gray_source; // Shallow copy.
    } else {
        // Sub-rectangle in input image coordinates.
        QRect relative_subrect(source_sub_rect);
        relative_subrect.moveTopLeft(
            source_sub_rect.topLeft() - source_rect.topLeft()
        );

        int const stride = gray_source.stride();
        int const offset = relative_subrect.top() * stride
                           + relative_subrect.left();

        trimmed_image = GrayImage(QImage(
                                      gray_source.data() + offset,
                                      relative_subrect.width(), relative_subrect.height(),
                                      stride, QImage::Format_Indexed8
                                  ));
    }

    status.throwIfCancelled();

    QSize const downscaled_size(to300dpi(trimmed_image.size(), m_dpi));

    // A 300dpi version of trimmed_image.
    GrayImage downscaled_input(
        scaleToGray(trimmed_image, downscaled_size)
    );
    trimmed_image = GrayImage(); // Save memory.

    status.throwIfCancelled();

    // Light areas indicate pictures.
    GrayImage picture_areas(detectPictures(downscaled_input, status, dbg));
    downscaled_input = GrayImage(); // Save memory.

    status.throwIfCancelled();

    BinaryThreshold const threshold(
        //BinaryThreshold::mokjiThreshold(picture_areas, 5, 26)
        48
    );

    // Scale back to original size.
    picture_areas = scaleToGray(
                        picture_areas, source_sub_rect.size()
                    );

    return BinaryImage(picture_areas, threshold);
}

void
OutputGenerator::modifyBinarizationMask(
    imageproc::BinaryImage& bw_mask,
    QRect const& mask_rect, ZoneSet const& zones, int filter) const
{
    QTransform xform(m_xform.transform());
    xform *= QTransform().translate(-mask_rect.x(), -mask_rect.y());

    typedef PictureLayerProperty PLP;

    // Pass 1: ERASER1
    if (filter & BINARIZATION_MASK_ERASER1) {
        for (Zone const& zone : zones) {
            if (zone.properties().locateOrDefault<PLP>()->layer() == PLP::ERASER1) {
                if (zone.type() == Zone::SplineType) {
                    QPolygonF const poly(zone.spline().toPolygon());
                    PolygonRasterizer::fill(bw_mask, BLACK, xform.map(poly), Qt::WindingFill);
                } else if (zone.type() == Zone::EllipseType) {
                    QPainterPath path;
                    QTransform t;
                    t.translate(zone.ellipse().center().x(), zone.ellipse().center().y());
                    t.rotate(zone.ellipse().angle());
                    t.translate(-zone.ellipse().center().x(), -zone.ellipse().center().y());
                    path.addEllipse(zone.ellipse().center(), zone.ellipse().rx(), zone.ellipse().ry());
                    path = t.map(path);
//                    path = xform.map(path);
                    PolygonRasterizer::fill(bw_mask, BLACK, xform.map(path.toFillPolygon()), Qt::WindingFill);
                }
            }
        }
    }

    // Pass 2: PAINTER2
    if (filter & BINARIZATION_MASK_PAINTER2) {
        for (Zone const& zone : zones) {
            if (zone.properties().locateOrDefault<PLP>()->layer() == PLP::PAINTER2) {
                if (zone.type() == Zone::SplineType) {
                    QPolygonF const poly(zone.spline().toPolygon());
                    PolygonRasterizer::fill(bw_mask, WHITE, xform.map(poly), Qt::WindingFill);
                } else if (zone.type() == Zone::EllipseType) {
                    QPainterPath path;
                    QTransform t;
                    t.translate(zone.ellipse().center().x(), zone.ellipse().center().y());
                    t.rotate(zone.ellipse().angle());
                    t.translate(-zone.ellipse().center().x(), -zone.ellipse().center().y());
                    path.addEllipse(zone.ellipse().center(), zone.ellipse().rx(), zone.ellipse().ry());
                    path = t.map(path);
//                    path = xform.map(path);
                    PolygonRasterizer::fill(bw_mask, WHITE, xform.map(path.toFillPolygon()), Qt::WindingFill);
                }
            }
        }
    }

    // Pass 1: ERASER3
    if (filter & BINARIZATION_MASK_ERASER3) {
        for (Zone const& zone : zones) {
            if (zone.properties().locateOrDefault<PLP>()->layer() == PLP::ERASER3) {
                if (zone.type() == Zone::SplineType) {
                    QPolygonF const poly(zone.spline().toPolygon());
                    PolygonRasterizer::fill(bw_mask, BLACK, xform.map(poly), Qt::WindingFill);
                } else if (zone.type() == Zone::EllipseType) {
                    QPainterPath path;
                    QTransform t;
                    t.translate(zone.ellipse().center().x(), zone.ellipse().center().y());
                    t.rotate(zone.ellipse().angle());
                    t.translate(-zone.ellipse().center().x(), -zone.ellipse().center().y());
                    path.addEllipse(zone.ellipse().center(), zone.ellipse().rx(), zone.ellipse().ry());
                    path = t.map(path);
//                    path = xform.map(path);
                    PolygonRasterizer::fill(bw_mask, BLACK, xform.map(path.toFillPolygon()), Qt::WindingFill);
                }
            }
        }
    }
}

QImage
OutputGenerator::processImpl(
    TaskStatus const& status, FilterData const& input,
//Quadro_Zoner
    //ZoneSet const& picture_zones, ZoneSet const& fill_zones,
    ZoneSet& picture_zones, ZoneSet const& fill_zones,
//Original_Foreground_Mixed
    bool keep_orig_fore_subscan,
    imageproc::BinaryImage* auto_layer_mask,
    imageproc::BinaryImage* speckles_image,
    DebugImages* const dbg,
    PageId* p_pageId,
    IntrusivePtr<Settings>* p_settings
) const
{
    RenderParams const render_params(m_colorParams);

//begin of modified by monday2000
//Original_Foreground_Mixed
//added:

    if (keep_orig_fore_subscan) {
        return processAsIs(input, status, fill_zones, dbg);
    }

    if (!render_params.whiteMargins()) {
        return processAsIs(input, status, fill_zones, dbg);
    } else {
        return processWithoutDewarping(
                   status, input, picture_zones, fill_zones,
                   auto_layer_mask, speckles_image, dbg,
                   p_pageId, p_settings
               );
    }
}

QImage
OutputGenerator::processAsIs(
    FilterData const& input, TaskStatus const& status,
    ZoneSet const& fill_zones,
    DebugImages* const dbg) const
{
    Q_UNUSED(dbg);

    uint8_t const dominant_gray = reserveBlackAndWhite<uint8_t>(
                                      calcDominantBackgroundGrayLevel(input.grayImage())
                                  );

    status.throwIfCancelled();

    QColor const bg_color(dominant_gray, dominant_gray, dominant_gray);

    QImage out;
    CommandLine const& cli = CommandLine::get();

    if (input.origImage().allGray() && !cli.hasTiffForceKeepColorSpace()
        #ifdef HAVE_EXIV2
            && !( GlobalStaticSettings::m_output_copy_icc_metadata &&
                  ImageMetadataCopier::iccProfileDefined(input.origImageFilename()) )
        #endif
            ) {
        if (m_outRect.isEmpty()) {
            QImage image(1, 1, QImage::Format_Indexed8);
            image.setColorTable(createGrayscalePalette());
            if (image.isNull()) {
                throw std::bad_alloc();
            }
            image.fill(dominant_gray);
            return image;
        }

        out = transformToGray(
                  input.grayImage(), m_xform.transform(), m_outRect,
                  OutsidePixels::assumeColor(bg_color)
              );
    } else {
        if (m_outRect.isEmpty()) {
            QImage image(1, 1, QImage::Format_RGB32);
            image.fill(bg_color.rgb());
            return image;
        }

        out = transform(
                  input.origImage(), m_xform.transform(), m_outRect,
                  OutsidePixels::assumeColor(bg_color)
              );
    }

    applyFillZonesInPlace(out, fill_zones);
    reserveBlackAndWhite(out);

    return out;
}

QImage
OutputGenerator::processWithoutDewarping(TaskStatus const& status, FilterData const& input,
        ZoneSet& picture_zones, ZoneSet const& fill_zones,
        imageproc::BinaryImage* auto_layer_mask,
        imageproc::BinaryImage* speckles_image,
        DebugImages* dbg, PageId* p_pageId,
        IntrusivePtr<Settings>* p_settings
                                        ) const
{
    RenderParams const render_params(m_colorParams);
    const bool suppress_smoothing = GlobalStaticSettings::m_disable_bw_smoothing &&
                                    (m_colorParams.colorMode() == ColorParams::BLACK_AND_WHITE);

    // The whole image minus the part cut off by the split line.
    QRect const big_margins_rect(
        m_xform.resultingPreCropArea().boundingRect().toRect() | m_contentRect
    );

    // For various reasons, we need some whitespace around the content
    // area.  This is the number of pixels of such whitespace.
//begin of modified by monday2000
//Marginal_Dewarping
    //int const content_margin = m_dpi.vertical() * 20 / 300;
    int const content_margin = 40;
//end of modified by monday2000

    // The content area (in output image coordinates) extended
    // with content_margin.  Note that we prevent that extension
    // from reaching the neighboring page.
    QRect const small_margins_rect(
        m_contentRect.adjusted(
            -content_margin, -content_margin,
            content_margin, content_margin
        ).intersected(big_margins_rect)
    );

    // This is the area we are going to pass to estimateBackground().
    // estimateBackground() needs some margins around content, and
    // generally smaller margins are better, except when there is
    // some garbage that connects the content to the edge of the
    // image area.
    QRect const normalize_illumination_rect(
#if 1
        small_margins_rect
#else
        big_margins_rect
#endif
    );

    QImage maybe_normalized;

    // Crop area in original image coordinates.
    QPolygonF const orig_image_crop_area(
        m_xform.transformBack().map(
            m_xform.resultingPreCropArea()
        )
    );

    // Crop area in maybe_normalized image coordinates.
    QPolygonF normalize_illumination_crop_area(m_xform.resultingPreCropArea());
    normalize_illumination_crop_area.translate(-normalize_illumination_rect.topLeft());

    if (render_params.normalizeIllumination() || render_params.mixedOutput()) {
        maybe_normalized = normalizeIlluminationGray(
                               status, input.grayImage(), orig_image_crop_area,
                               m_xform.transform(), normalize_illumination_rect, 0, dbg
                           );
    } else {
        maybe_normalized = transform(
                               input.origImage(), m_xform.transform(),
                               normalize_illumination_rect, OutsidePixels::assumeColor(Qt::white)
                           );
    }

    status.throwIfCancelled();

    QImage maybe_smoothed;

    // We only do smoothing if we are going to do binarization later.
    if (!render_params.needBinarization() || suppress_smoothing) {
        maybe_smoothed = maybe_normalized;
    } else {
        maybe_smoothed =  smoothToGrayscale(maybe_normalized, m_dpi);
        if (dbg) {
            dbg->add(maybe_smoothed, "smoothed");
        }
    }

    status.throwIfCancelled();

    if (render_params.binaryOutput() || m_outRect.isEmpty()) {
        BinaryImage dst(m_outRect.size().expandedTo(QSize(1, 1)), WHITE);

        if (!m_contentRect.isEmpty()) {
            BinaryImage bw_content(
                binarize(maybe_smoothed, normalize_illumination_crop_area)
            );
            if (dbg) {
                dbg->add(bw_content, "binarized_and_cropped");
            }

            status.throwIfCancelled();

            if (!suppress_smoothing) {
                morphologicalSmoothInPlace(bw_content, status);
                if (dbg) {
                    dbg->add(bw_content, "edges_smoothed");
                }
            }

            status.throwIfCancelled();

            QRect const src_rect(m_contentRect.translated(-normalize_illumination_rect.topLeft()));
            QRect const dst_rect(m_contentRect);
            rasterOp<RopSrc>(dst, dst_rect, bw_content, src_rect.topLeft());
            bw_content.release(); // Save memory.

            // It's important to keep despeckling the very last operation
            // affecting the binary part of the output. That's because
            // we will be reconstructing the input to this despeckling
            // operation from the final output file.
            maybeDespeckleInPlace(
                dst, m_outRect, m_outRect, m_despeckleLevel,
                speckles_image, m_dpi, status, dbg
            );
        }

        applyFillZonesInPlace(dst, fill_zones);
        return dst.toQImage();
    }

    QSize const target_size(m_outRect.size().expandedTo(QSize(1, 1)));

    BinaryImage bw_mask;
    BinaryImage bw_auto_layer_mask;
    if (render_params.mixedOutput()) {
        // This block should go before the block with
        // adjustBrightnessGrayscale(), which may convert
        // maybe_normalized from grayscale to color mode.

        if (auto_layer_mask) {
            if (auto_layer_mask->size() != target_size) {
                BinaryImage(target_size).swap(*auto_layer_mask);
            }

            auto_layer_mask->fill(BLACK);
        }

        if (render_params.anyLayer()) {
            bw_mask = estimateBinarizationMask(
                          status, GrayImage(maybe_normalized),
                          normalize_illumination_rect,
                          small_margins_rect, dbg
                      );

            if (dbg) {
                dbg->add(bw_mask, "bw_mask");
            }

            //Picture_Shape
            if (render_params.pictureZonesLayer()) {
                if (!picture_zones.auto_zones_found()) {
                    std::vector<QRect> areas;
                    bw_mask.rectangularize(WHITE, areas, GlobalStaticSettings::m_picture_detection_sensitivity);

                    QTransform xform1(m_xform.transform());
                    xform1 *= QTransform().translate(-small_margins_rect.x(), -small_margins_rect.y());

                    QTransform inv_xform(xform1.inverted());

                    for (int i = 0; i < (int)areas.size(); i++) {
                        QRectF area0(areas[i]);
                        QPolygonF area1(area0);
                        QPolygonF area(inv_xform.map(area1));

                        Zone zone1(area);

                        picture_zones.add(zone1);
                    }

                    picture_zones.setPictureZonesSensitivity(GlobalStaticSettings::m_picture_detection_sensitivity);
                    (*p_settings)->setPictureZones(*p_pageId, picture_zones);
                }

            } else {
                picture_zones.remove_auto_zones();
                (*p_settings)->setPictureZones(*p_pageId, picture_zones);
            }

            if (render_params.foregroundLayer()) {
                bw_auto_layer_mask = bw_mask; // need it later
            }

            if (render_params.autoLayer()) {
                if (!m_contentRect.isEmpty() && !render_params.foregroundLayer()) {
                    // if foregroundLayer - will have to overwrite auto_layer_mask later
                    // so just not wasting time
                    QRect const src_rect(m_contentRect.translated(-small_margins_rect.topLeft()));
                    QRect const dst_rect(m_contentRect);
                    rasterOp<RopSrc>(*auto_layer_mask, dst_rect, bw_mask, src_rect.topLeft());
                }
            } else {
                bw_mask = BinaryImage(maybe_normalized.size(), BLACK);
            }
        } else {
            bw_mask = BinaryImage(maybe_normalized.size(), BLACK);
        }

        status.throwIfCancelled();
    }

    if ((render_params.normalizeIllumination() && !input.origImage().allGray())
            || render_params.mixedOutput()) {
        // in case of mixedOutput we normalized image for picture detection and now should
        // restoren non-normalized image if it has !normalizeIllumination()
        QImage tmp;
        if (!input.origImage().allGray()
#ifdef HAVE_EXIV2
                || ( GlobalStaticSettings::m_output_copy_icc_metadata &&
                 ImageMetadataCopier::iccProfileDefined(input.origImageFilename()) )
#endif
                ) {
            assert(maybe_normalized.format() == QImage::Format_Indexed8);
            tmp = (
                      transform(
                          input.origImage(), m_xform.transform(),
                          normalize_illumination_rect,
                          OutsidePixels::assumeColor(Qt::white)
                      )
                  );

            status.throwIfCancelled();

            if (render_params.normalizeIllumination()) {
                adjustBrightnessGrayscale(tmp, maybe_normalized);
            }
        } else {
            tmp = (
                      transform(
                          input.grayImage(), m_xform.transform(),
                          normalize_illumination_rect,
                          OutsidePixels::assumeColor(Qt::white)
                      )
                  );
            status.throwIfCancelled();
        }
        maybe_normalized = tmp;
        if (dbg) {
            dbg->add(maybe_normalized, "norm_illum_color");
        }

    }

    if (!render_params.mixedOutput()) {
        // It's "Color / Grayscale" mode, as we handle B/W above.
        reserveBlackAndWhite(maybe_normalized);
    } else {

        if (!render_params.foregroundLayer()) {
            modifyBinarizationMask(bw_mask, small_margins_rect, picture_zones);
            if (dbg) {
                dbg->add(bw_mask, "bw_mask with zones");
            }
        }

        BinaryImage bw_content(
            binarize(maybe_smoothed, normalize_illumination_crop_area, &bw_mask)
        );

        std::unique_ptr<BinaryImage> foreground_mask = nullptr;
        if (render_params.foregroundLayer() &&
                (m_colorParams.blackWhiteOptions().thresholdAdjustment()
                 != m_colorParams.blackWhiteOptions().thresholdForegroundAdjustment())) {
            const int adj = m_colorParams.blackWhiteOptions().thresholdForegroundAdjustment();
            foreground_mask.reset(new BinaryImage(binarize(maybe_smoothed, normalize_illumination_crop_area, &bw_mask, &adj)));
        }

        maybe_smoothed = QImage(); // Save memory.
        if (dbg) {
            dbg->add(bw_content, "binarized_and_cropped");
        }

        status.throwIfCancelled();

        if (!suppress_smoothing) {
            morphologicalSmoothInPlace(bw_content, status);
            if (dbg) {
                dbg->add(bw_content, "edges_smoothed");
            }
            if (foreground_mask) {
                morphologicalSmoothInPlace(*foreground_mask, status);
            }
        }

        status.throwIfCancelled();

        // We don't want speckles in non-B/W areas, as they would
        // then get visualized on the Despeckling tab.
        rasterOp<RopAnd<RopSrc, RopDst> >(bw_content, bw_mask);
        if (foreground_mask) {
            rasterOp<RopAnd<RopSrc, RopDst> >(*foreground_mask, bw_mask);
        }

        status.throwIfCancelled();

        // It's important to keep despeckling the very last operation
        // affecting the binary part of the output. That's because
        // we will be reconstructing the input to this despeckling
        // operation from the final output file.
        maybeDespeckleInPlace(
            bw_content, small_margins_rect, m_contentRect,
            m_despeckleLevel, speckles_image, m_dpi, status, dbg
        );

        if (foreground_mask) {
            maybeDespeckleInPlace(
                *foreground_mask, small_margins_rect, m_contentRect,
                m_despeckleLevel, speckles_image, m_dpi, status, nullptr
            );
        }

        status.throwIfCancelled();

        if (render_params.foregroundLayer()) {
            if (foreground_mask) {
                bw_mask = foreground_mask->release();
                foreground_mask.reset(nullptr);
            } else {
                bw_mask = bw_content;
            }
            bw_mask.invert();

            BinaryImage new_auto_layer_mask = bw_mask;
            if (render_params.autoLayer()) {
                rasterOp<RopAnd<RopSrc, RopDst> >(new_auto_layer_mask, bw_auto_layer_mask);

                modifyBinarizationMask(bw_auto_layer_mask, small_margins_rect, picture_zones, BINARIZATION_MASK_ERASER1 | BINARIZATION_MASK_PAINTER2);
                rasterOp<RopAnd<RopSrc, RopDst> >(bw_mask, bw_auto_layer_mask);
                modifyBinarizationMask(bw_mask, small_margins_rect, picture_zones, BINARIZATION_MASK_ERASER3);
                bw_auto_layer_mask.release();
            } else {
                // apply all zones directly to color layer mask as we have no autolayer.
                modifyBinarizationMask(bw_mask, small_margins_rect, picture_zones);
            }

            if (!m_contentRect.isEmpty()) {
                QRect const src_rect(m_contentRect.translated(-small_margins_rect.topLeft()));
                QRect const dst_rect(m_contentRect);
                rasterOp<RopSrc>(*auto_layer_mask, dst_rect, new_auto_layer_mask, src_rect.topLeft());
            }

//            bw_content.fill(WHITE);
        }

        if (maybe_normalized.format() == QImage::Format_Indexed8) {
            combineMixed<uint8_t>(
                maybe_normalized, bw_content, bw_mask
            );
        } else {
            assert(maybe_normalized.format() == QImage::Format_RGB32
                   || maybe_normalized.format() == QImage::Format_ARGB32);

            combineMixed<uint32_t>(
                maybe_normalized, bw_content, bw_mask
            );
        }
    }

    status.throwIfCancelled();

    assert(!target_size.isEmpty());
    QImage dst(target_size, maybe_normalized.format());

    if (maybe_normalized.format() == QImage::Format_Indexed8) {
        dst.setColorTable(createGrayscalePalette());
        // White.  0xff is reserved if in "Color / Grayscale" mode.
        uint8_t const color = render_params.mixedOutput() ? 0xff : 0xfe;
        dst.fill(color);
    } else {
        // White.  0x[ff]ffffff is reserved if in "Color / Grayscale" mode.
        uint32_t const color = render_params.mixedOutput() ? 0xffffffff : 0xfffefefe;
        dst.fill(color);
    }

    if (dst.isNull()) {
        // Both the constructor and setColorTable() above can leave the image null.
        throw std::bad_alloc();
    }

    if (!m_contentRect.isEmpty()) {
        QRect const src_rect(m_contentRect.translated(-small_margins_rect.topLeft()));
        QRect const dst_rect(m_contentRect);
        drawOver(dst, dst_rect, maybe_normalized, src_rect);
    }

    applyFillZonesInPlace(dst, fill_zones);
    return dst;
}

QSize
OutputGenerator::from300dpi(QSize const& size, Dpi const& target_dpi)
{
    double const hscale = target_dpi.horizontal() / 300.0;
    double const vscale = target_dpi.vertical() / 300.0;
    int const width = qRound(size.width() * hscale);
    int const height = qRound(size.height() * vscale);
    return QSize(std::max(1, width), std::max(1, height));
}

QSize
OutputGenerator::to300dpi(QSize const& size, Dpi const& source_dpi)
{
    double const hscale = 300.0 / source_dpi.horizontal();
    double const vscale = 300.0 / source_dpi.vertical();
    int const width = qRound(size.width() * hscale);
    int const height = qRound(size.height() * vscale);
    return QSize(std::max(1, width), std::max(1, height));
}

QImage
OutputGenerator::convertToRGBorRGBA(QImage const& src)
{
    QImage::Format const fmt = src.hasAlphaChannel()
                               ? QImage::Format_ARGB32 : QImage::Format_RGB32;

    return src.convertToFormat(fmt);
}

void
OutputGenerator::fillMarginsInPlace(
    QImage& image, QPolygonF const& content_poly, QColor const& color)
{
    if (image.format() == QImage::Format_Indexed8 && image.isGrayscale()) {
        PolygonRasterizer::grayFillExcept(
            image, qGray(color.rgb()), content_poly, Qt::WindingFill
        );
        return;
    }

    assert(image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32);

    if (image.format() == QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    {
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(color);
        painter.setPen(Qt::NoPen);

        QPainterPath outer_path;
        outer_path.addRect(image.rect());
        QPainterPath inner_path;
        inner_path.addPolygon(content_poly);

        painter.drawPath(outer_path.subtracted(inner_path));
    }

    if (image.format() == QImage::Format_ARGB32_Premultiplied) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
}

GrayImage
OutputGenerator::detectPictures(
    GrayImage const& input_300dpi, TaskStatus const& status,
    DebugImages* const dbg)
{
    // We stretch the range of gray levels to cover the whole
    // range of [0, 255].  We do it because we want text
    // and background to be equally far from the center
    // of the whole range.  Otherwise text printed with a big
    // font will be considered a picture.
    GrayImage stretched(stretchGrayRange(input_300dpi, 0.01, 0.01));
    if (dbg) {
        dbg->add(stretched, "stretched");
    }

    status.throwIfCancelled();

    GrayImage eroded(erodeGray(stretched, QSize(3, 3), 0x00));
    if (dbg) {
        dbg->add(eroded, "eroded");
    }

    status.throwIfCancelled();

    GrayImage dilated(dilateGray(stretched, QSize(3, 3), 0xff));
    if (dbg) {
        dbg->add(dilated, "dilated");
    }

    stretched = GrayImage(); // Save memory.

    status.throwIfCancelled();

    grayRasterOp<CombineInverted>(dilated, eroded);
    GrayImage gray_gradient(dilated);
    dilated = GrayImage();
    eroded = GrayImage();
    if (dbg) {
        dbg->add(gray_gradient, "gray_gradient");
    }

    status.throwIfCancelled();

    GrayImage marker(erodeGray(gray_gradient, QSize(35, 35), 0x00));
    if (dbg) {
        dbg->add(marker, "marker");
    }

    status.throwIfCancelled();

    seedFillGrayInPlace(marker, gray_gradient, CONN8);
    gray_gradient = GrayImage(); // Save memory.
    GrayImage reconstructed(marker);
    marker = GrayImage(); // Save memory.

    if (dbg) {
        dbg->add(reconstructed, "reconstructed");
    }

    status.throwIfCancelled();

    grayRasterOp<GRopInvert<GRopSrc> >(reconstructed, reconstructed);
    if (dbg) {
        dbg->add(reconstructed, "reconstructed_inverted");
    }

    status.throwIfCancelled();

    GrayImage holes_filled(createFramedImage(reconstructed.size()));
    seedFillGrayInPlace(holes_filled, reconstructed, CONN8);
    reconstructed = GrayImage();
    if (dbg) {
        dbg->add(holes_filled, "holes_filled");
    }

    return holes_filled;
}

QImage
OutputGenerator::smoothToGrayscale(QImage const& src, Dpi const& dpi)
{
    int const min_dpi = std::min(dpi.horizontal(), dpi.vertical());
    int window;
    int degree;
    if (min_dpi <= 200) {
        window = 5;
        degree = 3;
    } else if (min_dpi <= 400) {
        window = 7;
        degree = 4;
    } else if (min_dpi <= 800) {
        window = 11;
        degree = 4;
    } else {
        window = 11;
        degree = 2;
    }
    return savGolFilter(src, QSize(window, window), degree, degree);
}

BinaryThreshold
OutputGenerator::adjustThreshold(BinaryThreshold threshold, const int* adjustment) const
{
    int adjusted = threshold;
    if (!adjustment) {
        adjusted += m_colorParams.blackWhiteOptions().thresholdAdjustment();
    } else {
        adjusted += *adjustment;
    }

    // Hard-bounding threshold values is necessary for example
    // if all the content went into the picture mask.
    return BinaryThreshold(qBound(30, adjusted, 225));
}

BinaryThreshold
OutputGenerator::calcBinarizationThreshold(
    QImage const& image, BinaryImage const& mask) const
{
    GrayscaleHistogram hist(image, mask);
    return adjustThreshold(BinaryThreshold::otsuThreshold(hist));
}

BinaryThreshold
OutputGenerator::calcBinarizationThreshold(
    QImage const& image, QPolygonF const& crop_area, BinaryImage const* mask) const
{
    QPainterPath path;
    path.addPolygon(crop_area);

    if (path.contains(image.rect())) {
        return adjustThreshold(BinaryThreshold::otsuThreshold(image));
    } else {
        BinaryImage modified_mask(image.size(), BLACK);
        PolygonRasterizer::fillExcept(modified_mask, WHITE, crop_area, Qt::WindingFill);
        modified_mask = erodeBrick(modified_mask, QSize(3, 3), WHITE);

        if (mask) {
            rasterOp<RopAnd<RopSrc, RopDst> >(modified_mask, *mask);
        }

        return calcBinarizationThreshold(image, modified_mask);
    }
}

BinaryImage
OutputGenerator::binarize(QImage const& image, BinaryImage const& mask, const int* adjustment) const
{
    BlackWhiteOptions const& black_white_options = m_colorParams.blackWhiteOptions();
    ThresholdFilter const thresholdMethod = black_white_options.thresholdMethod();

    BinaryImage binarized;
    if ((image.format() == QImage::Format_Mono) || (image.format() == QImage::Format_MonoLSB))
    {
        binarized = BinaryImage(image);
    }
    else
    {
        switch (thresholdMethod)
        {
        case OTSU:
        {
            GrayscaleHistogram hist(image, mask);
            BinaryThreshold const bw_thresh(BinaryThreshold::otsuThreshold(hist));
            binarized = BinaryImage(image, adjustThreshold(bw_thresh, adjustment));
            break;
        }
        case SAUVOLA:
        {
            int const threshold_delta = black_white_options.thresholdSauvolaAdjustment();
            QSize const window_size = QSize(black_white_options.thresholdSauvolaWindowSize(), black_white_options.thresholdSauvolaWindowSize());
            double const threshold_coef = black_white_options.thresholdSauvolaCoef();
            binarized = binarizeSauvola(image, window_size, threshold_coef, threshold_delta);
            break;
        }
        case WOLF:
        {
            int const threshold_delta = black_white_options.thresholdWolfAdjustment();
            QSize const window_size = QSize(black_white_options.thresholdWolfWindowSize(), black_white_options.thresholdWolfWindowSize());
            double const threshold_coef = black_white_options.thresholdWolfCoef();
            binarized = binarizeWolf(image, window_size, 1, 254, threshold_coef, threshold_delta);
            break;
        }
        case GATOS:
        {
            int const threshold_delta = black_white_options.thresholdGatosAdjustment();
            QSize const window_size = QSize(black_white_options.thresholdGatosWindowSize(), black_white_options.thresholdGatosWindowSize());
            double const threshold_coef = black_white_options.thresholdGatosCoef();
            double const threshold_scale = black_white_options.thresholdGatosScale();
            binarized = binarizeGatos(image, window_size, threshold_scale, 3.0, threshold_coef, threshold_delta);
            break;
        }
        }
    }

    // Fill masked out areas with white.
    rasterOp<RopAnd<RopSrc, RopDst> >(binarized, mask);

    return binarized;
}

BinaryImage
OutputGenerator::binarize(QImage const& image,
                          QPolygonF const& crop_area, BinaryImage const* mask, const int* adjustment) const
{
    BinaryImage modified_mask(image.size(), BLACK);
    PolygonRasterizer::fillExcept(modified_mask, WHITE, crop_area, Qt::WindingFill);
    modified_mask = erodeBrick(modified_mask, QSize(3, 3), WHITE);

    if (mask) {
        rasterOp<RopAnd<RopSrc, RopDst> >(modified_mask, *mask);
    }

    return binarize(image, modified_mask, adjustment);
}

/**
 * \brief Remove small connected components that are considered to be garbage.
 *
 * Both the size and the distance to other components are taken into account.
 *
 * \param[in,out] image The image to despeckle.
 * \param image_rect The rectangle corresponding to \p image in the same
 *        coordinate system where m_contentRect and m_cropRect are defined.
 * \param mask_rect The area within the image to consider.  Defined not
 *        relative to \p image, but in the same coordinate system where
 *        m_contentRect and m_cropRect are defined.  This only affects
 *        \p speckles_img, if provided.
 * \param level Despeckling aggressiveness.
 * \param speckles_img If provided, the removed black speckles will be written
 *        there.  The speckles image is always considered to correspond
 *        to m_cropRect, so it will have the size of m_cropRect.size().
 *        Only the area within \p mask_rect will be copied to \p speckles_img.
 *        The rest will be filled with white.
 * \param dpi The DPI of the input image.  See the note below.
 * \param status Task status.
 * \param dbg An optional sink for debugging images.
 *
 * \note This function only works effectively when the DPI is symmetric,
 * that is, its horizontal and vertical components are equal.
 */
void
OutputGenerator::maybeDespeckleInPlace(
    imageproc::BinaryImage& image,
    QRect const& image_rect, QRect const& mask_rect,
    DespeckleLevel const level, BinaryImage* speckles_img,
    Dpi const& dpi, TaskStatus const& status, DebugImages* dbg) const
{
    QRect const src_rect(mask_rect.translated(-image_rect.topLeft()));
    QRect const dst_rect(mask_rect);

    if (speckles_img) {
        BinaryImage(m_outRect.size(), WHITE).swap(*speckles_img);
        if (!mask_rect.isEmpty()) {
            rasterOp<RopSrc>(*speckles_img, dst_rect, image, src_rect.topLeft());
        }
    }

    if (level != DESPECKLE_OFF) {
        Despeckle::Level lvl = Despeckle::NORMAL;
        switch (level) {
        case DESPECKLE_CAUTIOUS:
            lvl = Despeckle::CAUTIOUS;
            break;
        case DESPECKLE_NORMAL:
            lvl = Despeckle::NORMAL;
            break;
        case DESPECKLE_AGGRESSIVE:
            lvl = Despeckle::AGGRESSIVE;
            break;
        default:;
        }

        Despeckle::despeckleInPlace(image, dpi, lvl, status, dbg);

        if (dbg) {
            dbg->add(image, "despeckled");
        }
    }

    if (speckles_img) {
        if (!mask_rect.isEmpty()) {
            rasterOp<RopSubtract<RopDst, RopSrc> >(
                *speckles_img, dst_rect, image, src_rect.topLeft()
            );
        }
    }
}

void
OutputGenerator::morphologicalSmoothInPlace(
    BinaryImage& bin_img, TaskStatus const& status)
{
    // When removing black noise, remove small ones first.

    {
        char const pattern[] =
            "XXX"
            " - "
            "   ";
        hitMissReplaceAllDirections(bin_img, pattern, 3, 3);
    }

    status.throwIfCancelled();

    {
        char const pattern[] =
            "X ?"
            "X  "
            "X- "
            "X- "
            "X  "
            "X ?";
        hitMissReplaceAllDirections(bin_img, pattern, 3, 6);
    }

    status.throwIfCancelled();

    {
        char const pattern[] =
            "X ?"
            "X ?"
            "X  "
            "X- "
            "X- "
            "X- "
            "X  "
            "X ?"
            "X ?";
        hitMissReplaceAllDirections(bin_img, pattern, 3, 9);
    }

    status.throwIfCancelled();

    {
        char const pattern[] =
            "XX?"
            "XX?"
            "XX "
            "X+ "
            "X+ "
            "X+ "
            "XX "
            "XX?"
            "XX?";
        hitMissReplaceAllDirections(bin_img, pattern, 3, 9);
    }

    status.throwIfCancelled();

    {
        char const pattern[] =
            "XX?"
            "XX "
            "X+ "
            "X+ "
            "XX "
            "XX?";
        hitMissReplaceAllDirections(bin_img, pattern, 3, 6);
    }

    status.throwIfCancelled();

    {
        char const pattern[] =
            "   "
            "X+X"
            "XXX";
        hitMissReplaceAllDirections(bin_img, pattern, 3, 3);
    }
}

void
OutputGenerator::hitMissReplaceAllDirections(
    imageproc::BinaryImage& img, char const* const pattern,
    int const pattern_width, int const pattern_height)
{
    hitMissReplaceInPlace(img, WHITE, pattern, pattern_width, pattern_height);

    std::vector<char> pattern_data(pattern_width * pattern_height, ' ');
    char* const new_pattern = &pattern_data[0];

    // Rotate 90 degrees clockwise.
    char const* p = pattern;
    int new_width = pattern_height;
    int new_height = pattern_width;
    for (int y = 0; y < pattern_height; ++y) {
        for (int x = 0; x < pattern_width; ++x, ++p) {
            int const new_x = pattern_height - 1 - y;
            int const new_y = x;
            new_pattern[new_y * new_width + new_x] = *p;
        }
    }
    hitMissReplaceInPlace(img, WHITE, new_pattern, new_width, new_height);

    // Rotate upside down.
    p = pattern;
    new_width = pattern_width;
    new_height = pattern_height;
    for (int y = 0; y < pattern_height; ++y) {
        for (int x = 0; x < pattern_width; ++x, ++p) {
            int const new_x = pattern_width - 1 - x;
            int const new_y = pattern_height - 1 - y;
            new_pattern[new_y * new_width + new_x] = *p;
        }
    }
    hitMissReplaceInPlace(img, WHITE, new_pattern, new_width, new_height);

    // Rotate 90 degrees counter-clockwise.
    p = pattern;
    new_width = pattern_height;
    new_height = pattern_width;
    for (int y = 0; y < pattern_height; ++y) {
        for (int x = 0; x < pattern_width; ++x, ++p) {
            int const new_x = y;
            int const new_y = pattern_width - 1 - x;
            new_pattern[new_y * new_width + new_x] = *p;
        }
    }
    hitMissReplaceInPlace(img, WHITE, new_pattern, new_width, new_height);
}

QSize
OutputGenerator::calcLocalWindowSize(Dpi const& dpi)
{
    QSizeF const size_mm(3, 30);
    QSizeF const size_inch(size_mm * constants::MM2INCH);
    QSizeF const size_pixels_f(
        dpi.horizontal() * size_inch.width(),
        dpi.vertical() * size_inch.height()
    );
    QSize size_pixels(size_pixels_f.toSize());

    if (size_pixels.width() < 3) {
        size_pixels.setWidth(3);
    }
    if (size_pixels.height() < 3) {
        size_pixels.setHeight(3);
    }

    return size_pixels;
}

unsigned char
OutputGenerator::calcDominantBackgroundGrayLevel(QImage const& img)
{
    // TODO: make a color version.
    // In ColorPickupInteraction.cpp we have code for median color finding.
    // We can use that.

    QImage const gray(toGrayscale(img));

    BinaryImage mask(binarizeOtsu(gray));
    mask.invert();

    GrayscaleHistogram const hist(gray, mask);

    int integral_hist[256];
    integral_hist[0] = hist[0];
    for (int i = 1; i < 256; ++i) {
        integral_hist[i] = hist[i] + integral_hist[i - 1];
    }

    int const num_colors = 256;
    int const window_size = 10;

    int best_pos = 0;
    int best_sum = integral_hist[window_size - 1];
    for (int i = 1; i <= num_colors - window_size; ++i) {
        int const sum = integral_hist[i + window_size - 1] - integral_hist[i - 1];
        if (sum > best_sum) {
            best_sum = sum;
            best_pos = i;
        }
    }

    int half_sum = 0;
    for (int i = best_pos; i < best_pos + window_size; ++i) {
        half_sum += hist[i];
        if (half_sum >= best_sum / 2) {
            return i;
        }
    }

    assert(!"Unreachable");
    return 0;
}

void
OutputGenerator::applyFillZonesInPlace(
    QImage& img, ZoneSet const& zones,
    boost::function<QPointF(QPointF const&)> const& orig_to_output) const
{
    if (zones.empty()) {
        return;
    }

    QImage canvas(img.convertToFormat(QImage::Format_ARGB32_Premultiplied));

    {
        QPainter painter(&canvas);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);

        for (Zone const& zone : zones) {
            QColor const color(zone.properties().locateOrDefault<FillColorProperty>()->color());
            painter.setBrush(color);
            if (zone.type() == Zone::SplineType) {
                QPolygonF const poly(zone.spline().transformed(orig_to_output).toPolygon());
                painter.drawPolygon(poly, Qt::WindingFill);
            } else if (zone.type() == Zone::EllipseType) {
                const SerializableEllipse e = zone.ellipse().transformed(orig_to_output);
                QPainterPath path;
                QTransform t;
                t.translate(e.center().x(), e.center().y());
                t.rotate(e.angle());
                t.translate(-e.center().x(), -e.center().y());
                path.addEllipse(e.center(), e.rx(), e.ry());
                path = t.map(path);
                painter.drawPolygon(path.toFillPolygon(), Qt::WindingFill);
            }
        }
    }

    if (img.format() == QImage::Format_Indexed8 && img.isGrayscale()) {
        img = toGrayscale(canvas);
    } else {
        img = canvas.convertToFormat(img.format());
    }
}

/**
 * A simplified version of the above, using toOutput() for translation
 * from original image to output image coordinates.
 */
void
OutputGenerator::applyFillZonesInPlace(QImage& img, ZoneSet const& zones) const
{
    typedef QPointF(QTransform::*MapPointFunc)(QPointF const&) const;
    applyFillZonesInPlace(
        img, zones, boost::bind((MapPointFunc)&QTransform::map, m_xform.transform(), _1)
    );
}

void
OutputGenerator::applyFillZonesInPlace(
    imageproc::BinaryImage& img, ZoneSet const& zones,
    boost::function<QPointF(QPointF const&)> const& orig_to_output) const
{
    if (zones.empty()) {
        return;
    }

    for (Zone const& zone : zones) {
        QColor const color(zone.properties().locateOrDefault<FillColorProperty>()->color());
        BWColor const bw_color = qGray(color.rgb()) < 128 ? BLACK : WHITE;
        if (zone.type() == Zone::SplineType) {
            QPolygonF const poly(zone.spline().transformed(orig_to_output).toPolygon());
            PolygonRasterizer::fill(img, bw_color, poly, Qt::WindingFill);
        } else if (zone.type() == Zone::EllipseType) {
            const SerializableEllipse e = zone.ellipse().transformed(orig_to_output);
            QPainterPath path;
            QTransform t;
            t.translate(e.center().x(), e.center().y());
            t.rotate(e.angle());
            t.translate(-e.center().x(), -e.center().y());
            path.addEllipse(e.center(), e.rx(), e.ry());
            path = t.map(path);
            PolygonRasterizer::fill(img, bw_color, path.toFillPolygon(), Qt::WindingFill);
        }
    }
}

/**
 * A simplified version of the above, using toOutput() for translation
 * from original image to output image coordinates.
 */
void
OutputGenerator::applyFillZonesInPlace(
    imageproc::BinaryImage& img, ZoneSet const& zones) const
{
    typedef QPointF(QTransform::*MapPointFunc)(QPointF const&) const;
    applyFillZonesInPlace(
        img, zones, boost::bind((MapPointFunc)&QTransform::map, m_xform.transform(), _1)
    );
}

} // namespace output
