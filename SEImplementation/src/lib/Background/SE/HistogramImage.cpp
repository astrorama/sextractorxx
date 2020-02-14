/** Copyright © 2019 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3.0 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <Histogram/Histogram.h> // From Alexandria

#include "SEFramework/Image/ImageChunk.h"
#include "SEImplementation/Background/SE/HistogramImage.h"
#include "SEImplementation/Background/SE/KappaSigmaBinning.h"


using Euclid::Histogram::Histogram;

namespace SourceXtractor {

template<typename T>
HistogramImage<T>::HistogramImage(
  const std::shared_ptr<Image<T>>& image, const std::shared_ptr<Image<T>>& variance, T var_threshold,
  int cell_w, int cell_h,
  T invalid_value, T kappa1, T kappa2, T kappa3,
  T rtol, size_t max_iter): m_image(image), m_variance(variance), m_variance_threshold(var_threshold),
                            m_cell_w(cell_w), m_cell_h(cell_h),
                            m_invalid(invalid_value),
                            m_kappa1(kappa1), m_kappa2(kappa2), m_kappa3(kappa3),
                            m_rtol(rtol), m_max_iter(max_iter) {
  auto hist_width = std::div(image->getWidth(), m_cell_w);
  if (hist_width.rem)
    ++hist_width.quot;
  auto hist_height = std::div(image->getHeight(), m_cell_h);
  if (hist_height.rem)
    ++hist_height.quot;

  m_mode = VectorImage<T>::create(hist_width.quot, hist_height.quot);
  m_sigma = VectorImage<T>::create(hist_width.quot, hist_height.quot);
  if (m_variance) {
    m_weight_mode = VectorImage<T>::create(hist_width.quot, hist_height.quot);
    m_weight_sigma = VectorImage<T>::create(hist_width.quot, hist_height.quot);
  }

  // Initialize grid
  for (int y = 0; y < m_mode->getHeight(); ++y) {
    for (int x = 0; x < m_mode->getWidth(); ++x) {
      processCell(x, y);
    }
  }
}

template<typename T>
std::shared_ptr<Image<T>> HistogramImage<T>::getModeImage() const {
  return m_mode;
}

template<typename T>
std::shared_ptr<Image<T>> HistogramImage<T>::getVarianceImage() const {
  return m_sigma;
}

template<typename T>
std::shared_ptr<Image<T>> HistogramImage<T>::getWeightImage() const {
  return m_weight_mode;
}

template <typename T>
std::tuple<T, T> HistogramImage<T>::getBackGuess(const std::vector<T> &data) const {
  Histogram<T> histo(KappaSigmaBinning(m_kappa1, m_kappa2), data.begin(), data.end());

  T mean, median, sigma;
  std::tie(mean, median, sigma) = histo.getStats();
  T prev_sigma = sigma * 10;

  assert(!std::isnan(mean));

  for (size_t iter = 0; iter < m_max_iter && sigma > 0.1 && std::abs(sigma / prev_sigma - 1.0) > m_rtol; ++iter) {
    histo.clip(median - sigma * m_kappa3, median + sigma * m_kappa3);
    std::tie(mean, median, sigma) = histo.getStats();
  }

  // Sigma is 0
  T mode;
  if (std::abs(sigma) <= 0) {
    mode = mean;
  }
  // Not crowded: mean and median do not differ more than 30%
  else if (std::abs((mean - median) / sigma) < 0.3) {
    mode = 2.5 * median - 1.5 * mean;
  }
  // Crowded case: we use the median
  else {
    mode = median;
  }
  return std::make_tuple(mode, sigma);
}

template<typename T>
void HistogramImage<T>::processCell(int x, int y) {
  int off_x = x * m_cell_w;
  int off_y = y * m_cell_h;
  int w = std::min(m_cell_w, m_image->getWidth() - off_x);
  int h = std::min(m_cell_h, m_image->getHeight() - off_y);

  auto img_chunk = VectorImage<T>::create(m_image->getChunk(off_x, off_y, w, h));
  auto& data = img_chunk->getData();

  std::vector<T> filtered;
  filtered.reserve(data.size());

  if (m_weight_mode) {
    std::vector<T> filtered_weight;
    filtered_weight.reserve(data.size());

    auto variance_chunk = VectorImage<T>::create(m_variance->getChunk(off_x, off_y, w, h));
    auto& variance_data = variance_chunk->getData();
    for (size_t i = 0; i < data.size(); ++i) {
      if (data[i] != m_invalid && variance_data[i] < m_variance_threshold) {
        filtered.emplace_back(data[i]);
        filtered_weight.emplace_back(variance_data[i]);
      }
    }

    T wmode, wsigma;
    std::tie(wmode, wsigma) = getBackGuess(filtered_weight);
    m_weight_mode->setValue(x, y, wmode);
    m_weight_sigma->setValue(x, y, wsigma*wsigma);
  }
  else {
    for (size_t i =0; i < data.size(); ++i) {
      if (data[i] != m_invalid)
        filtered.emplace_back(data[i]);
    }
  }

  T mode, sigma;
  std::tie(mode, sigma) = getBackGuess(filtered);
  m_mode->setValue(x, y, mode);
  m_sigma->setValue(x, y, sigma*sigma);
}

template
class HistogramImage<SeFloat>;

} // end of namespace SourceXtractor
