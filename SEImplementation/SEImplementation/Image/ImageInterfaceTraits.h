/*
 * ImageInterfaceTraits.h
 *
 *  Created on: May 3, 2017
 *      Author: mschefer
 */

#ifndef _SEIMPLEMENTATION_IMAGE_IMAGEINTERFACETRAITS_H_
#define _SEIMPLEMENTATION_IMAGE_IMAGEINTERFACETRAITS_H_

#define INTERP_MAXKERNELWIDTH 8 // Max. range of kernel (pixels)

#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "ModelFitting/Image/ImageTraits.h"
#include "SEFramework/Image/Image.h"
#include "SEFramework/Image/VectorImage.h"
#include "SEFramework/Image/MultiplyImage.h"

#ifdef __APPLE__
#define sincosf __sincosf
#endif

namespace ModelFitting {

using ImageInterfaceType = SExtractor::VectorImage<SExtractor::SeFloat>;
using ImageInterfaceTypePtr = std::shared_ptr<ImageInterfaceType>;
using WriteableInterfaceType = SExtractor::WriteableImage<SExtractor::SeFloat>;
using WriteableInterfaceTypePtr = std::shared_ptr<WriteableInterfaceType>;

template <>
struct ImageTraits<ImageInterfaceTypePtr> {

  using iterator = std::vector<ImageInterfaceType::PixelType>::iterator;

  static ImageInterfaceTypePtr factory(std::size_t width, std::size_t height) {
    return SExtractor::VectorImage<ImageInterfaceType::PixelType>::create(width, height);
  }

  static std::size_t width(const ImageInterfaceTypePtr& image) {
    return image->getWidth();
  }

  static std::size_t height(const ImageInterfaceTypePtr& image) {
    return image->getHeight();
  }

  static ImageInterfaceType::PixelType& at(ImageInterfaceTypePtr& image, std::size_t x, std::size_t y) {
    return image->at(x, y);
  }

  static ImageInterfaceType::PixelType at(const ImageInterfaceTypePtr& image, std::size_t x, std::size_t y) {
    return image->at(x, y);
  }

  static iterator begin(const ImageInterfaceTypePtr& image) {
    return image->getData().begin();
  }

  static iterator end(const ImageInterfaceTypePtr& image) {
    return image->getData().end();
  }

  static void addImageToImage(ImageInterfaceTypePtr& target_image, const ImageInterfaceTypePtr& source_image,
                              double scale_factor, double x, double y);
}; // end of class ImageTraits<ImageInterfaceTypePtr>

} // end of namespace ModelFitting

namespace SExtractor {

// Make those types available to the SExtractor namespace for convenience
using ModelFitting::ImageInterfaceType;
using ModelFitting::ImageInterfaceTypePtr;
using ModelFitting::WriteableInterfaceType;
using ModelFitting::WriteableInterfaceTypePtr;

// Interpolation types
typedef enum  {INTERP_NEARESTNEIGHBOUR, INTERP_BILINEAR, INTERP_LANCZOS2,
    INTERP_LANCZOS3, INTERP_LANCZOS4}       interpenum;

float interpolate_pix(float *pix, float x, float y,
                             int xsize, int ysize, interpenum interptype);

} // end of namespace SExtractor

#endif /* _SEIMPLEMENTATION_IMAGE_IMAGEINTERFACETRAITS_H_ */
