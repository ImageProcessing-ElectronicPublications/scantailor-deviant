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

#ifndef OUTPUT_OUTPUTGENERATOR_H_
#define OUTPUT_OUTPUTGENERATOR_H_

#include "imageproc/Connectivity.h"
#include "Dpi.h"
#include "ColorParams.h"
#include "Params.h"
#include "DespeckleLevel.h"
#include "ImageTransformation.h"
#ifndef Q_MOC_RUN
#include <boost/function.hpp>
#endif
#include <QSize>
#include <QRect>
#include <QTransform>
#include <QColor>
#include <QPointF>
#include <QLineF>
#include <QPolygonF>
#include <vector>
#include <utility>
#include <stdint.h>
//begin of modified by monday2000
//Picture_Shape
#include "Params.h"
//Quadro_Zoner
#include "PageId.h"
#include "IntrusivePtr.h"
#include "Settings.h"
//Marginal_Dewarping
#include <QMessageBox>
#include "TiffWriter.h"
#include <QtCore/qmath.h>
#include <QFile>
#include "imageproc/SkewFinder.h"
//end of modified by monday2000

class TaskStatus;
class DebugImages;
class FilterData;
class ZoneSet;
class QSize;
class QImage;

namespace imageproc
{
class BinaryImage;
class BinaryThreshold;
class GrayImage;
}

namespace dewarping
{
class DistortionModel;
class CylindricalSurfaceDewarper;
}
//begin of modified by monday2000
//Marginal_Dewarping
using namespace imageproc;
//end of modified by monday2000
namespace output
{

enum BinarizationMask {
    BINARIZATION_MASK_ERASER1 = 1,
    BINARIZATION_MASK_PAINTER2 = 2,
    BINARIZATION_MASK_ERASER3 = 4
};

class OutputGenerator
{
public:
    OutputGenerator(
        Dpi const& dpi, ColorParams const& color_params,
        DespeckleLevel despeckle_level,
        ImageTransformation const& xform,
        QPolygonF const& content_rect_phys);

    /**
     * \brief Produce the output image.
     *
     * \param status For asynchronous task cancellation.
     * \param input The input image plus data produced by previous stages.
     * \param picture_zones A set of manual picture zones.
     * \param fill_zones A set of manual fill zones.
     * \param distortion_model A curved rectangle.
     * \param auto_picture_mask If provided, the auto-detected picture mask
     *        will be written there.  It would only happen if automatic picture
     *        detection actually took place.  Otherwise, nothing will be
     *        written into the provided image.  Black areas on the mask
     *        indicate pictures.  The manual zones aren't represented in it.
     * \param speckles_image If provided, the speckles removed from the
     *        binarized image will be written there.  It would only happen
     *        if despeckling was required and actually took place.
     *        Otherwise, nothing will be written into the provided image.
     *        Because despeckling is intentionally the last operation on
     *        the B/W part of the image, the "pre-despeckle" image may be
     *        restored from the output and speckles images, allowing despeckling
     *        to be performed again with different settings, without going
     *        through the whole output generation process again.
     * \param dbg An optional sink for debugging images.
     */
    QImage process(
        TaskStatus const& status, FilterData const& input,
//Quadro_Zoner
        ZoneSet& picture_zones, ZoneSet const& fill_zones,
//Original_Foreground_Mixed
        bool keep_orig_fore_subscan = false,
        imageproc::BinaryImage* auto_picture_mask = 0,
        imageproc::BinaryImage* speckles_image = 0,
        DebugImages* dbg = 0,
        PageId* p_pageId = nullptr, IntrusivePtr<Settings>* p_settings = nullptr
    ) const;

    QSize outputImageSize() const;

    /**
     * \brief Returns the content rectangle in output image coordinates.
     */
    QRect outputContentRect() const;
private:

    QImage processImpl(
        TaskStatus const& status, FilterData const& input,
//Quadro_Zoner
        //ZoneSet const& picture_zones, ZoneSet const& fill_zones,
        ZoneSet& picture_zones, ZoneSet const& fill_zones,
//Original_Foreground_Mixed
        bool keep_orig_fore_subscan = false,
        imageproc::BinaryImage* auto_layer_mask = 0,
        imageproc::BinaryImage* speckles_image = 0,
        DebugImages* dbg = 0,
        PageId* p_pageId = nullptr, IntrusivePtr<Settings>* p_settings = nullptr
    ) const;

    QImage processAsIs(
        FilterData const& input, TaskStatus const& status,
        ZoneSet const& fill_zones,
        DebugImages* dbg = 0) const;

    QImage processWithoutDewarping(TaskStatus const& status, FilterData const& input,
//Quadro_Zoner
                                   //ZoneSet const& picture_zones, ZoneSet const& fill_zones,
                                   ZoneSet& picture_zones, ZoneSet const& fill_zones,
                                   imageproc::BinaryImage* auto_layer_mask = 0,
                                   imageproc::BinaryImage* speckles_image = 0,
//Picture_Shape
                                   DebugImages* dbg = 0,
                                   PageId* p_pageId = nullptr, IntrusivePtr<Settings>* p_settings = nullptr
                                  ) const;

    static QSize from300dpi(QSize const& size, Dpi const& target_dpi);

    static QSize to300dpi(QSize const& size, Dpi const& source_dpi);

    static QImage convertToRGBorRGBA(QImage const& src);

    static void fillMarginsInPlace(
        QImage& image, QPolygonF const& content_poly, QColor const& color);

    static imageproc::GrayImage normalizeIlluminationGray(
        TaskStatus const& status,
        QImage const& input, QPolygonF const& area_to_consider,
        QTransform const& xform, QRect const& target_rect,
        imageproc::GrayImage* background = 0, DebugImages* dbg = 0);

    static imageproc::GrayImage detectPictures(
        imageproc::GrayImage const& input_300dpi, TaskStatus const& status,
        DebugImages* dbg = 0);

    imageproc::BinaryImage estimateBinarizationMask(
        TaskStatus const& status, imageproc::GrayImage const& gray_source,
        QRect const& source_rect, QRect const& source_sub_rect,
        DebugImages* const dbg) const;

    void modifyBinarizationMask(
        imageproc::BinaryImage& bw_mask,
        QRect const& mask_rect, ZoneSet const& zones,
        int filter = BINARIZATION_MASK_ERASER1 | BINARIZATION_MASK_PAINTER2 | BINARIZATION_MASK_ERASER3) const;

    imageproc::BinaryThreshold adjustThreshold(imageproc::BinaryThreshold threshold, const int* adjustment = nullptr) const;

    imageproc::BinaryThreshold calcBinarizationThreshold(
        QImage const& image, imageproc::BinaryImage const& mask) const;

    imageproc::BinaryThreshold calcBinarizationThreshold(
        QImage const& image, QPolygonF const& crop_area,
        imageproc::BinaryImage const* mask = 0) const;

    imageproc::BinaryImage binarize(QImage const& image, imageproc::BinaryImage const& mask, const int* adjustment = nullptr) const;

    imageproc::BinaryImage binarize(QImage const& image, QPolygonF const& crop_area,
                                    imageproc::BinaryImage const* mask = 0, const int* adjustment = nullptr) const;

    void maybeDespeckleInPlace(
        imageproc::BinaryImage& image, QRect const& image_rect,
        QRect const& mask_rect, DespeckleLevel level,
        imageproc::BinaryImage* speckles_img,
        Dpi const& dpi, TaskStatus const& status, DebugImages* dbg) const;

    static QImage smoothToGrayscale(QImage const& src, Dpi const& dpi);

    static void morphologicalSmoothInPlace(
        imageproc::BinaryImage& img, TaskStatus const& status);

    static void hitMissReplaceAllDirections(
        imageproc::BinaryImage& img, char const* pattern,
        int pattern_width, int pattern_height);

    static QSize calcLocalWindowSize(Dpi const& dpi);

    static unsigned char calcDominantBackgroundGrayLevel(QImage const& img);

    static QImage normalizeIllumination(QImage const& gray_input, DebugImages* dbg);

    QImage transformAndNormalizeIllumination(
        QImage const& gray_input, DebugImages* dbg,
        QImage const* morph_background = 0) const;

    QImage transformAndNormalizeIllumination2(
        QImage const& gray_input, DebugImages* dbg,
        QImage const* morph_background = 0) const;

    void applyFillZonesInPlace(QImage& img, ZoneSet const& zones,
                               boost::function<QPointF(QPointF const&)> const& orig_to_output) const;

    void applyFillZonesInPlace(QImage& img, ZoneSet const& zones) const;

    void applyFillZonesInPlace(imageproc::BinaryImage& img, ZoneSet const& zones,
                               boost::function<QPointF(QPointF const&)> const& orig_to_output) const;

    void applyFillZonesInPlace(imageproc::BinaryImage& img, ZoneSet const& zones) const;

    Dpi m_dpi;
    ColorParams m_colorParams;

    /**
     * Transformation from the input to the output image coordinates.
     */
    ImageTransformation m_xform;

    /**
     * The rectangle corresponding to the output image.
     * The top-left corner will always be at (0, 0).
     */
    QRect m_outRect;

    /**
     * The content rectangle in output image coordinates.
     */
    QRect m_contentRect;

    DespeckleLevel m_despeckleLevel;
};

} // namespace output

#endif
