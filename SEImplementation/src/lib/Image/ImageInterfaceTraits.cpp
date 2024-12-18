/*
 * ImageInterfaceTraits.cpp
 *
 *  Created on: Aug 16, 2019
 *      Author: mschefer
 */

#include <iostream>

#include "SEImplementation/Image/ImageInterfaceTraits.h"

namespace SourceXtractor {

using Traits = ModelFitting::ImageTraits<ImageInterfaceTypePtr>;
constexpr float PI = boost::math::constants::pi<float>();

static void makeLanczos2Kernel(float pos, float *kernel, const float threshold) {
  float x, val, sinx1, cosx1;

  if (pos < threshold && pos > -threshold) {
    *(kernel++) = 0.0;
    *(kernel++) = 1.0;
    *(kernel++) = 0.0;
    *kernel = 0.0;
  }
  else {
    x = -PI / 2.0 * (pos + 1.0);
    sincosf(x, &sinx1, &cosx1);
    val = (*(kernel++) = sinx1 / (x * x));
    x += PI / 2.0;
    val += (*(kernel++) = -cosx1 / (x * x));
    x += PI / 2.0;
    val += (*(kernel++) = -sinx1 / (x * x));
    x += PI / 2.0;
    val += (*kernel = cosx1 / (x * x));
    val = 1.0 / val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *kernel *= val;
  }
}

static void makeLanczos3Kernel(float pos, float *kernel, const float threshold) {
  float x, val, sinx1, sinx2, sinx3, cosx1;

  if (pos < threshold && pos > -threshold) {
    *(kernel++) = 0.0;
    *(kernel++) = 0.0;
    *(kernel++) = 1.0;
    *(kernel++) = 0.0;
    *(kernel++) = 0.0;
    *kernel = 0.0;
  }
  else {
    x = -PI / 3.0 * (pos + 2.0);
    sincosf(x, &sinx1, &cosx1);
    val = (*(kernel++) = sinx1 / (x * x));
    x += PI / 3.0;
    val += (*(kernel++) = (sinx2 = -0.5 * sinx1 - 0.866025403785 * cosx1) / (x * x));
    x += PI / 3.0;
    val += (*(kernel++) = (sinx3 = -0.5 * sinx1 + 0.866025403785 * cosx1) / (x * x));
    x += PI / 3.0;
    val += (*(kernel++) = sinx1 / (x * x));
    x += PI / 3.0;
    val += (*(kernel++) = sinx2 / (x * x));
    x += PI / 3.0;
    val += (*kernel = sinx3 / (x * x));
    val = 1.0 / val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *kernel *= val;
  }
}

static void makeLanczos4Kernel(float pos, float *kernel, const float threshold) {
  float x, val, sinx1, sinx2, sinx3, cosx1;

  if (pos < threshold && pos > -threshold) {
    *(kernel++) = 0.0;
    *(kernel++) = 0.0;
    *(kernel++) = 0.0;
    *(kernel++) = 1.0;
    *(kernel++) = 0.0;
    *(kernel++) = 0.0;
    *(kernel++) = 0.0;
    *kernel = 0.0;
  }
  else {
    x = -PI / 4.0 * (pos + 3.0);
    sincosf(x, &sinx1, &cosx1);
    val = (*(kernel++) = sinx1 / (x * x));
    x += PI / 4.0;
    val += (*(kernel++) = -(sinx2 = 0.707106781186 * (sinx1 + cosx1)) / (x * x));
    x += PI / 4.0;
    val += (*(kernel++) = cosx1 / (x * x));
    x += PI / 4.0;
    val += (*(kernel++) = -(sinx3 = 0.707106781186 * (cosx1 - sinx1)) / (x * x));
    x += PI / 4.0;
    val += (*(kernel++) = -sinx1 / (x * x));
    x += PI / 4.0;
    val += (*(kernel++) = sinx2 / (x * x));
    x += PI / 4.0;
    val += (*(kernel++) = -cosx1 / (x * x));
    x += PI / 4.0;
    val += (*kernel = sinx3 / (x * x));
    val = 1.0 / val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *(kernel--) *= val;
    *kernel *= val;
  }
}

inline void make_kernel(float pos, float *kernel, interpenum interptype) {
  const float threshold = 1e-6;

  switch (interptype) {
    case INTERP_NEARESTNEIGHBOUR:
      *kernel = 1.0;
      break;
    case INTERP_BILINEAR:
      *(kernel++) = 1.0 - pos;
      *kernel = pos;
      break;
    case INTERP_LANCZOS2:
      makeLanczos2Kernel(pos, kernel, threshold);
      break;
    case INTERP_LANCZOS3:
      makeLanczos3Kernel(pos, kernel, threshold);
      break;
    case INTERP_LANCZOS4:
      makeLanczos4Kernel(pos, kernel, threshold);
      break;

  }
}


float interpolate_pix(float *pix, float x, float y,
                      int xsize, int ysize, interpenum interptype) {

  static const int interp_kernwidth[5] = {1, 2, 4, 6, 8};

  float buffer[INTERP_MAXKERNELWIDTH],
    kernel[INTERP_MAXKERNELWIDTH],
    *kvector, *pixin, *pixout,
    dx, dy, val;
  int i, j, ix, iy, kwidth, step;

  kwidth = interp_kernwidth[interptype];

  //-- Get the integer part of the current coordinate or nearest neighbour
  if (interptype == INTERP_NEARESTNEIGHBOUR) {
    ix = (int) (x - 0.50001);
    iy = (int) (y - 0.50001);
  }
  else {
    ix = (int) x;
    iy = (int) y;
  }

  //-- Store the fractional part of the current coordinate
  dx = x - ix;
  dy = y - iy;
  //-- Check if interpolation start/end exceed image boundary
  ix -= kwidth / 2;
  iy -= kwidth / 2;
  if (ix < 0 || ix + kwidth <= 0 || ix + kwidth > xsize ||
      iy < 0 || iy + kwidth <= 0 || iy + kwidth > ysize) {
    return 0.0;
  }

  //-- First step: interpolate along NAXIS1 from the data themselves
  make_kernel(dx, kernel, interptype);
  step = xsize - kwidth;
  pixin = pix + iy * xsize + ix; // Set starting pointer
  pixout = buffer;
  for (j = kwidth; j--;) {
    val = 0.0;
    kvector = kernel;
    for (i = kwidth; i--;) {
      val += *(kvector++) * *(pixin++);
    }
    *(pixout++) = val;
    pixin += step;
  }

  //-- Second step: interpolate along NAXIS2 from the interpolation buffer
  make_kernel(dy, kernel, interptype);
  pixin = buffer;
  val = 0.0;
  kvector = kernel;
  for (i = kwidth; i--;) {
    val += *(kvector++) * *(pixin++);
  }

  return val;
}


inline double getClamped(const ImageInterfaceTypePtr& image, int x, int y) {
  return Traits::at(image, std::max(0, std::min(x, (int) Traits::width(image) - 1)),
                    std::max(0, std::min(y, (int) Traits::height(image) - 1)));
}

void shiftResize(const ImageInterfaceTypePtr& source, ImageInterfaceTypePtr& window,
                 double scale_factor, double x_shift, double y_shift) {
  int window_width = Traits::width(window);
  int window_height = Traits::height(window);
  for (int x_win = 0; x_win < window_width; x_win++) {
    for (int y_win = 0; y_win < window_height; y_win++) {
      double x = (x_win + 0.5 - x_shift) / scale_factor - 0.5;
      double y = (y_win + 0.5 - y_shift) / scale_factor - 0.5;

      int xi = std::floor(x);
      int yi = std::floor(y);

      double x_delta = x - xi;
      double y_delta = y - yi;

      double v00 = getClamped(source, xi, yi);
      double v01 = getClamped(source, xi, yi + 1);
      double v10 = getClamped(source, xi + 1, yi);
      double v11 = getClamped(source, xi + 1, yi + 1);

      Traits::at(window, x_win, y_win) = (1.0 - y_delta) * ((1.0 - x_delta) * v00 + x_delta * v10) +
                                         y_delta * ((1.0 - x_delta) * v01 + x_delta * v11);
    }
  }
}

void shiftResizeLancszos(const ImageInterfaceTypePtr& source, ImageInterfaceTypePtr& window,
                         double scale_factor, double x_shift, double y_shift) {
  int window_width = Traits::width(window);
  int window_height = Traits::height(window);

  auto data = &source->getData()[0];
  auto source_width = source->getWidth();
  auto source_height = source->getHeight();

  for (int x_win = 0; x_win < window_width; x_win++) {
    float x = (x_win + 0.5 - x_shift) / scale_factor + 0.5;
    for (int y_win = 0; y_win < window_height; y_win++) {
      float y = (y_win + 0.5 - y_shift) / scale_factor + 0.5;
      Traits::at(window, x_win, y_win) = interpolate_pix(data, x, y, source_width, source_height,
                                                         INTERP_LANCZOS4);
    }
  }
}

void shiftResizeLancszosFast(const ImageInterfaceTypePtr& source, ImageInterfaceTypePtr& window,
                             double scale_factor, double x_shift, double y_shift) {
  int window_width = Traits::width(window);
  int window_height = Traits::height(window);

  //auto data = &source->getData()[0];
  auto source_width = source->getWidth();
  auto source_height = source->getHeight();

  //
  float kernel[8];

  // First resize vertically and store the result in a transposed buffer
  auto buffer = Traits::factory(window_height, source_width);
  for (unsigned int buff_x = 0; buff_x < Traits::width(buffer); buff_x++) {
    float pos = (buff_x + 0.5 - y_shift) / scale_factor + 0.5;
    int ipos = int(pos) - 4;

    if (ipos < 0 || ipos + 7 >= source_height) {
      continue;
    }

    make_kernel(pos - int(pos), kernel, INTERP_LANCZOS4);
    for (unsigned int buff_y = 0; buff_y < Traits::height(buffer); buff_y++) {
      float val = 0.f;
      for (int i = 0; i < 8; i++) {
        val += kernel[i] * Traits::at(source, buff_y, ipos + i);
      }
      Traits::at(buffer, buff_x, buff_y) = val;
    }
  }

  // resize on the other axis
  for (int x_win = 0; x_win < window_width; x_win++) {
    float pos = (x_win + 0.5 - x_shift) / scale_factor + 0.5;
    int ipos = int(pos) - 4;
    if (ipos < 0 || ipos + 7 >= source_width) {
      continue;
    }
    make_kernel(pos - int(pos), kernel, INTERP_LANCZOS4);

    for (int y_win = 0; y_win < window_height; y_win++) {
      float val = 0.f;
      for (int i = 0; i < 8; i++) {
        val += kernel[i] * Traits::at(buffer, y_win, ipos + i);
      }
      Traits::at(window, x_win, y_win) = val;
    }
  }
}


}

namespace ModelFitting {

// Adds the source_image to the target image scaled by scale_factor and centered at x, y
void ModelFitting::ImageTraits<ImageInterfaceTypePtr>::addImageToImage(
  ImageInterfaceTypePtr& target_image, const ImageInterfaceTypePtr& source_image,
  double scale_factor, double x, double y) {
  // Calculate the size in pixels of the image2 after in the scale of image1
  double scaled_width = width(source_image) * scale_factor;
  double scaled_height = height(source_image) * scale_factor;
  // Calculate the window of the image1 which is affected
  int x_min = std::floor(x - scaled_width / 2.);
  int x_max = std::ceil(x + scaled_width / 2.);
  int window_width = x_max - x_min;
  int y_min = std::floor(y - scaled_height / 2.);
  int y_max = std::ceil(y + scaled_height / 2.);
  int window_height = y_max - y_min;
  // Calculate the shift of the image2 inside the window
  double x_shift = x - scaled_width / 2. - x_min;
  double y_shift = y - scaled_height / 2. - y_min;
  // Create the scaled and shifted window
  auto window = factory(window_width, window_height);

  shiftResizeLancszosFast(source_image, window, scale_factor, x_shift, y_shift);

  // We need to correct the window for the scaling, so it has the same integral
  // with the image2
  double corr_factor = 1. / (scale_factor * scale_factor);
  // Add the window to the image1
  for (int x_im = std::max(x_min, 0); x_im < std::min<int>(x_max, width(target_image)); ++x_im) {
    for (int y_im = std::max(y_min, 0); y_im < std::min<int>(y_max, height(target_image)); ++y_im) {
      int x_win = x_im - x_min;
      int y_win = y_im - y_min;
      at(target_image, x_im, y_im) += corr_factor * at(window, x_win, y_win);
    }
  }
}

}

