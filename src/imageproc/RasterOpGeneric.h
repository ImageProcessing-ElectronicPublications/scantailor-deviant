/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimich@gmail.com>

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

#ifndef IMAGEPROC_RASTER_OP_GENERIC_H_
#define IMAGEPROC_RASTER_OP_GENERIC_H_

#include "foundation/IndexSequence.h"
#include "foundation/GridAccessor.h"
#include "BinaryImage.h"
#include <QSize>
#include <stdint.h>
#include <assert.h>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <stdexcept>

namespace imageproc
{

/**
 * \brief Perform an operation on a single image.
 *
 * \param data The pointer to image data.
 * \param stride The number of elements of type T per image line.
 * \param size Image size.
 * \param operation An operation to perform.  It will be called like this:
 * \code
 * operation(data[offset]);
 * \endcode
 * Depending on whether T is const, the operation may be able to modify the image.
 * Hinst: boost::lambda is an easy way to construct operations.
 */
template<typename T, typename Op>
void rasterOpGeneric(T* data, int stride, QSize size, Op operation);

/**
 * \brief Perform an operation on a pair of images.
 *
 * \param data1 The pointer to image data of the first image.
 * \param stride1 The number of elements of type T1 per line of the first image.
 * \param size Dimensions of both images.
 * \param data2 The pointer to image data of the second image.
 * \param stride2 The number of elements of type T2 per line of the second image.
 * \param operation An operation to perform.  It will be called like this:
 * \code
 * operation(data1[offset1], data2[offset2]);
 * \endcode
 * Depending on whether T1 / T2 are const, the operation may be able to modify
 * one or both of them.
 * Hinst: boost::lambda is an easy way to construct operations.
 */
template<typename T1, typename T2, typename Op>
void rasterOpGeneric(T1* data1, int stride1, QSize size,
                     T2* data2, int stride2, Op operation);

/**
 * \brief Same as the one above, except one of the images is a const BinaryImage.
 *
 * \p operation will be called like this:
 * \code
 * uint32_t const bit1 = <something> ? 1 : 0;
 * operation(bitl, data2[offset2]);
 * \endcode
 */
template<typename T2, typename Op>
void rasterOpGeneric(BinaryImage const& image1, T2* data2, int stride2, Op operation);

/**
 * \brief Same as the one above, except one of the images is a non-const BinaryImage.
 *
 * \p operation will be called like this:
 * \code
 * BitProxy bit1(...);
 * operation(bitl, data2[offset2]);
 * \endcode
 * BitProxy will have implicit conversion to uint32_t returning 0 or 1,
 * and an assignment operator from uint32_t, expecting 0 or 1 only.
 */
template<typename T2, typename Op>
void rasterOpGeneric(BinaryImage const& image1, T2* data2, int stride2, Op operation);

/*======================== Implementation ==========================*/

template<typename T, typename Op>
void rasterOpGeneric(T* data, int stride, QSize size, Op operation)
{
    if (size.isEmpty()) {
        return;
    }

    int const w = size.width();
    int const h = size.height();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            operation(data[x]);
        }
        data += stride;
    }
}

template<typename T1, typename T2, typename Op>
void rasterOpGeneric(T1* data1, int stride1, QSize size,
                     T2* data2, int stride2, Op operation)
{
    if (size.isEmpty()) {
        return;
    }

    int const w = size.width();
    int const h = size.height();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            operation(data1[x], data2[x]);
        }
        data1 += stride1;
        data2 += stride2;
    }
}

template<typename T2, typename Op>
void rasterOpGeneric(BinaryImage const& image1, T2* data2, int stride2, Op operation)
{
    if (image1.isNull()) {
        return;
    }

    int const w = image1.width();
    int const h = image1.height();
    int const stride1 = image1.wordsPerLine();
    uint32_t const* data1 = image1.data();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int const shift = 31 - (x & 31);
            operation((data1[x >> 5] >> shift) & uint32_t(1), data2[x]);
        }
        data1 += stride1;
        data2 += stride2;
    }
}

namespace rop_generic_impl
{

class BitProxy
{
public:
    BitProxy(uint32_t& word, int shift) : m_rWord(word), m_shift(shift) {}

    BitProxy(BitProxy const& other) : m_rWord(other.m_rWord), m_shift(other.m_shift) {}

    BitProxy& operator=(uint32_t bit)
    {
        assert(bit <= 1);
        uint32_t const mask = uint32_t(1) << m_shift;
        m_rWord = (m_rWord & ~mask) | (bit << m_shift);
        return *this;
    }

    operator uint32_t() const
    {
        return (m_rWord >> m_shift) & uint32_t(1);
    }
private:
    uint32_t& m_rWord;
    int m_shift;
};

} // namespace rop_generic_impl

template<typename T2, typename Op>
void rasterOpGeneric(BinaryImage& image1, T2* data2, int stride2, Op operation)
{
    using namespace rop_generic_impl;

    if (image1.isNull()) {
        return;
    }

    int const w = image1.width();
    int const h = image1.height();
    int const stride1 = image1.wordsPerLine();
    uint32_t* data1 = image1.data();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            BitProxy bit1(data1[x >> 5], 31 - (x & 31));
            operation(bit1, data2[x]);
        }
        data1 += stride1;
        data2 += stride2;
    }
}

/**
 * @brief Applies a user-provided function to pixels from one or more images.
 *
 * The user-provided function will be called for each pixel individually.
 * The function will be passed the number of arguments equal to the number
 * of images passed to rasterOpGeneric(). When more than one image is passed,
 * dimensions of all images have to match. Argument types generally correspond
 * to pixel types, except they can also be references to pixel types, allowing
 * image modifications. In some cases instead of a pixel type you have a proxy
 * object. That's the case with BinaryImage, where BWPixelProxy class represents
 * a pixel.
 * Example:
 * @code
 * GrayImage img1 = ...;
 * GrayImage img2 = ...;
 * rasterOpGeneric([](uint8_t& pix1, uint8_t pix2) {
 *     pix1 = std::max(pix1, pix2) - std::min(pix1, pix2);
 * }, img1, img2);
 * @endcode
 *
 * @par Extending For Custom Image Types
 * In order to make rasterOpGeneric() work with a custom image type, you need
 * to do one of the following:
 * -# Implement an accessor() method in the custom class (perhaps both const and
 *    non-const versions) returning an appropriate instantiation of GridAccessor
 *    template.
 * -# In the same namespace as the custom image class, declare and implement
 *    the following functions:
 * @code
 * CustomLineAccessorType createLineAccessor(CustomImage const& image);
 *
 * // Optionally another one like above, taking a non-const reference.
 *
 * std::tuple<int, int> extractDimensions(CustomImage const& image);
 * @endcode
 * The CustomLineAccessorType shall have an API like this:
 * @code
 * class CustomLineAccessorType
 * {
 * public:
 *     PixelType operator[](ptrdiff_t x) const;
 *
 *     void nextLine();
 * };
 * @endcode
 * Where the pixel type may be the actual type of a pixel, a const or non-const
 * reference to it, or a proxy class.
 *
 * @param op The function to call for each pixel.
 * @param images A variable list of arguments, each of them can be one of supported
 *        image classes or an instance of GridAccessor.
 */
template<typename Operation, typename... Images>
void rasterOpGeneric(Operation&& op, Images&&... images);

/**
 * @brief Applies a user-provided function to pixels from one or more images.
 *
 * rasterOpGenericXY() works just like rasterOpGeneric(), except the user-provided
 * function receives two extra arguments: the x and y coordinate of the pixel
 * being processed.
 */
template<typename Operation, typename... Images>
void rasterOpGenericXY(Operation&& op, Images&&... images);

namespace rop_generic
{

    template<typename T>
    class GridLineAccessor
    {
    public:
        explicit GridLineAccessor(GridAccessor<T> const& accessor)
            : m_pLine(accessor.data), m_stride(accessor.stride) {}

        inline T& operator[](ptrdiff_t x) const
        {
            return m_pLine[x];
        }

        void nextLine()
        {
            m_pLine += m_stride;
        }
    private:
        T* m_pLine;
        ptrdiff_t m_stride;
    };

    template<typename T>
    GridLineAccessor<T> createLineAccessor(GridAccessor<T> const& accessor)
    {
        return GridLineAccessor<T>(accessor);
    }

    template<typename Obj>
    auto createLineAccessor(Obj& object) ->
        GridLineAccessor<typename std::remove_reference<decltype(*object.accessor().data)>::type>
    {
        using T = typename std::remove_reference<decltype(*object.accessor().data)>::type;
        return GridLineAccessor<T>(object.accessor());
    }

    template<typename T>
    std::tuple<int, int> extractDimensions(GridAccessor<T> const& accessor)
    {
        return std::make_tuple(accessor.width, accessor.height);
    }

    template<typename T>
    std::tuple<int, int> extractDimensions(T& object)
    {
        return extractDimensions(object.accessor());
    }

    template<typename... Images>
    auto createLineAccessors(Images&&... images) ->
        decltype(std::make_tuple(createLineAccessor(images)...))
    {
        return std::make_tuple(createLineAccessor(images)...);
    }

    inline void validateDimensions(std::tuple<int, int> const& reference_dims)
    {
    }

    template<typename FirstImage, typename... Images>
    void validateDimensions(
        std::tuple<int, int> const& reference_dims,
        FirstImage const& first_image, Images const&... other_images)
    {
        if (reference_dims != extractDimensions(first_image))
        {
            throw std::invalid_argument("rasterOpGeneric: inconsistent image dimensions");
        }

        validateDimensions(reference_dims, other_images...);
    }

    template<typename FirstImage, typename... Images>
    std::tuple<int, int> getAndValidateDimensions(
        FirstImage const& first_image, Images const&... other_images)
    {
        std::tuple<int, int> const dims = extractDimensions(first_image);
        if (std::get<0>(dims) < 0 || std::get<1>(dims) < 0)
        {
            throw std::invalid_argument("rasterOpGeneric: invalid image dimensions");
        }

        validateDimensions(dims, other_images...);
        return dims;
    }

    template<typename Operation, typename Tuple, size_t... Idx>
    inline void performOperation(
        Operation&& op, Tuple const& line_accessors, ptrdiff_t x, IndexSequence<Idx...>)
    {
        op(std::get<Idx>(line_accessors)[x] ...);
    }

    template<typename Operation, typename Tuple, size_t... Idx>
    inline void performOperationXY(
        Operation&& op, Tuple const& line_accessors, ptrdiff_t x, ptrdiff_t y, IndexSequence<Idx...>)
    {
        op(std::get<Idx>(line_accessors)[x] ..., x, y);
    }

    inline void moveToNextLineImpl()
    {
    }

    template<typename LineAccessor, typename... LineAccessors>
    void moveToNextLineImpl(LineAccessor& first, LineAccessors&... others)
    {
        first.nextLine();
        moveToNextLineImpl(others...);
    }

    template<typename Tuple, size_t... Idx>
    void moveToNextLine(Tuple& line_accessors, IndexSequence<Idx...>)
    {
        moveToNextLineImpl(std::get<Idx>(line_accessors)...);
    }

} // namespace rop_generic


template<typename Operation, typename... Images>
void rasterOpGeneric(Operation&& op, Images&&... images)
{
    using namespace rop_generic;

    std::tuple<int, int> const dims = getAndValidateDimensions(images...);
    ptrdiff_t const width = std::get<0>(dims);
    ptrdiff_t const height = std::get<1>(dims);

    auto line_accessors_tuple = createLineAccessors(images...);
    typename IndexSequenceGenerator<sizeof...(Images)>::type seq;

    for (ptrdiff_t y = 0; y < height; ++y)
    {
        for (ptrdiff_t x = 0; x < width; ++x)
        {
            performOperation(op, line_accessors_tuple, x, seq);
        }
        moveToNextLine(line_accessors_tuple, seq);
    }
}

template<typename Operation, typename... Images>
void rasterOpGenericXY(Operation&& op, Images&&... images)
{
    using namespace rop_generic;

    std::tuple<int, int> const dims = getAndValidateDimensions(images...);
    ptrdiff_t const width = std::get<0>(dims);
    ptrdiff_t const height = std::get<1>(dims);

    auto line_accessors_tuple = createLineAccessors(images...);
    typename IndexSequenceGenerator<sizeof...(Images)>::type seq;

    for (ptrdiff_t y = 0; y < height; ++y)
    {
        for (ptrdiff_t x = 0; x < width; ++x)
        {
            performOperationXY(op, line_accessors_tuple, x, y, seq);
        }
        moveToNextLine(line_accessors_tuple, seq);
    }
}

} // namespace imageproc

#endif
