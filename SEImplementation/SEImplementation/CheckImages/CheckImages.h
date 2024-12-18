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
/*
 * CheckImages.h
 *
 *  Created on: May 30, 2017
 *      Author: mschefer
 */

#ifndef _SEIMPLEMENTATION_CHECKIMAGES_CHECKIMAGES_H_
#define _SEIMPLEMENTATION_CHECKIMAGES_CHECKIMAGES_H_

#include <mutex>
#include <map>
#include <iostream>

#include <boost/filesystem/path.hpp>

#include "SEFramework/Configuration/Configurable.h"
#include "SEFramework/CoordinateSystem/CoordinateSystem.h"
#include "SEFramework/Image/Image.h"
#include "SEFramework/Image/VectorImage.h"
#include "SEFramework/Image/ProcessedImage.h"
#include "SEFramework/Image/WriteableImage.h"
#include "SEFramework/Frame/Frame.h"

#include "SEImplementation/Image/LockedWriteableImage.h"


namespace SourceXtractor {

class CheckImages : public Configurable {

public:

  virtual ~CheckImages() = default;

  void saveImages();

  std::shared_ptr<WriteableImage<int>> getSegmentationImage(size_t index) const {
    if (index < m_segmentation_images.size()) {
      auto segmentation_image = m_segmentation_images.at(index);
      if (segmentation_image != nullptr) {
        return LockedWriteableImage<int>::create(segmentation_image);
      }
    }
    return nullptr;
  }

  std::shared_ptr<WriteableImage<int>> getPartitionImage(size_t index) const {
    if (index < m_partition_images.size()) {
      auto partition_image = m_partition_images.at(index);
      if (partition_image != nullptr) {
        return LockedWriteableImage<int>::create(partition_image);
      }
    }
    return nullptr;
  }

  std::shared_ptr<WriteableImage<int>> getGroupImage(size_t index) const {
    if (index < m_group_images.size()) {
      auto group_image = m_group_images.at(index);
      if (group_image != nullptr) {
        return LockedWriteableImage<int>::create(group_image);
      }
    }
    return nullptr;
  }

  std::shared_ptr<WriteableImage<int>> getDetectionAutoApertureImage(size_t index) const {
    if (index < m_auto_aperture_images.size()) {
      auto auto_aperture_image = m_auto_aperture_images.at(index);
      if (auto_aperture_image != nullptr) {
        return LockedWriteableImage<int>::create(auto_aperture_image);
      }
    }
    return nullptr;
  }

  std::shared_ptr<WriteableImage<int>> getDetectionApertureImage(size_t index) const {
    if (index < m_aperture_images.size()) {
      auto aperture_image = m_aperture_images.at(index);
      if (aperture_image != nullptr) {
        return LockedWriteableImage<int>::create(aperture_image);
      }
    }
    return nullptr;
  }

  std::shared_ptr<WriteableImage<SeFloat>> getMoffatImage(size_t index) const {
    if (index < m_moffat_images.size()) {
      auto moffat_image = m_moffat_images.at(index);
      if (moffat_image != nullptr) {
        return LockedWriteableImage<SeFloat>::create(moffat_image);
      }
    }
    return nullptr;
  }

  std::shared_ptr<WriteableImage<int>> getMeasurementAutoApertureImage(unsigned int frame_number);

  std::shared_ptr<WriteableImage<int>> getMeasurementApertureImage(unsigned int frame_number);

  std::shared_ptr<WriteableImage<MeasurementImage::PixelType>> getModelFittingImage(unsigned int frame_number);

  std::shared_ptr<WriteableImage<int>> getFittingWindowImage(unsigned int frame_number);

  std::shared_ptr<WriteableImage<MeasurementImage::PixelType>> getPsfImage(unsigned int frame_number);

  std::shared_ptr<WriteableImage<float>> getMLDetectionImage(unsigned int plane_number, size_t index);

  void addBackgroundCheckImage(std::shared_ptr<Image<SeFloat>> background_image) {
    m_background_images.emplace_back(background_image);
  }

  void addMeasurementBackgroundCheckImage(size_t index, std::shared_ptr<Image<SeFloat>> background_image) {
    m_measurement_background_images[index] = background_image;
  }

  void addVarianceCheckImage(std::shared_ptr<Image<SeFloat>> variance_image) {
    m_variance_images.emplace_back(variance_image);
  }

  void addMeasurementVarianceCheckImage(size_t index, std::shared_ptr<Image<SeFloat>> variance_image) {
    m_measurement_variance_images[index] = variance_image;
  }

  void addFilteredCheckImage(std::shared_ptr<Image<SeFloat>> filtered_image) {
    m_filtered_images.emplace_back(filtered_image);
  }

  void addThresholdedCheckImage(std::shared_ptr<Image<SeFloat>> thresholded_image) {
    m_thresholded_images.emplace_back(thresholded_image);
  }

  void addSnrCheckImage(std::shared_ptr<Image<SeFloat>> snr_image) {
    m_snr_images.emplace_back(snr_image);
  }

  std::shared_ptr<WriteableImage<SeFloat>> getWriteableCheckImage(std::string id, int width, int height);
  void setCustomCheckImage(std::string id, std::shared_ptr<Image<SeFloat>> image);

  void reportConfigDependencies(Euclid::Configuration::ConfigManager& manager) const override;
  void configure(Euclid::Configuration::ConfigManager& manager) override;

  static CheckImages& getInstance() {
    if (m_instance == nullptr) {
      m_instance.reset(new CheckImages);
    }

    return *m_instance;
  }

private:
  CheckImages();

  static std::unique_ptr<CheckImages> m_instance;

  struct FrameInfo {
    std::string m_label;
    int m_width, m_height;
    std::shared_ptr<CoordinateSystem> m_coordinate_system;
    std::shared_ptr<DetectionImage> m_subtracted_image;
  };

  // check image
  std::vector<std::shared_ptr<WriteableImage<int>>> m_segmentation_images;
  std::vector<std::shared_ptr<WriteableImage<int>>> m_partition_images;
  std::vector<std::shared_ptr<WriteableImage<int>>> m_group_images;
  std::vector<std::shared_ptr<WriteableImage<int>>> m_auto_aperture_images;
  std::vector<std::shared_ptr<WriteableImage<int>>> m_aperture_images;
  std::vector<std::shared_ptr<WriteableImage<SeFloat>>> m_moffat_images;

  std::map<unsigned int, std::shared_ptr<WriteableImage<int>>> m_measurement_aperture_images;
  std::map<unsigned int, std::shared_ptr<WriteableImage<int>>> m_measurement_auto_aperture_images;
  std::map<unsigned int, std::shared_ptr<WriteableImage<MeasurementImage::PixelType>>> m_check_image_model_fitting, m_check_image_psf;
  std::map<unsigned int, std::shared_ptr<WriteableImage<int>>> m_check_image_fitting_window;
  std::vector<std::map<unsigned int, std::shared_ptr<WriteableImage<float>>>> m_check_image_ml_detection;

  std::vector<std::shared_ptr<DetectionImage>> m_detection_images;
  std::vector<std::shared_ptr<Image<SeFloat>>> m_background_images;
  std::vector<std::shared_ptr<Image<SeFloat>>> m_filtered_images;
  std::vector<std::shared_ptr<Image<SeFloat>>> m_thresholded_images;
  std::vector<std::shared_ptr<Image<SeFloat>>> m_snr_images;
  std::vector<std::shared_ptr<WeightImage>> m_variance_images;

  std::map<unsigned int, std::shared_ptr<Image<SeFloat>>> m_measurement_background_images;
  std::map<unsigned int, std::shared_ptr<WeightImage>> m_measurement_variance_images;


  std::vector<std::shared_ptr<CoordinateSystem>> m_coordinate_systems;

  boost::filesystem::path m_model_fitting_image_filename;
  boost::filesystem::path m_fitting_window_image_filename;
  boost::filesystem::path m_residual_filename;
  boost::filesystem::path m_background_filename;
  boost::filesystem::path m_variance_filename;
  boost::filesystem::path m_segmentation_filename;
  boost::filesystem::path m_partition_filename;
  boost::filesystem::path m_group_filename;
  boost::filesystem::path m_filtered_filename;
  boost::filesystem::path m_thresholded_filename;
  boost::filesystem::path m_snr_filename;
  boost::filesystem::path m_auto_aperture_filename;
  boost::filesystem::path m_aperture_filename;
  boost::filesystem::path m_moffat_filename;
  boost::filesystem::path m_psf_filename;
  boost::filesystem::path m_ml_detection_filename;

  boost::filesystem::path m_measurement_background_filename;
  boost::filesystem::path m_measurement_variance_filename;

  std::map<boost::filesystem::path, std::tuple<std::shared_ptr<Image<SeFloat>>, bool>> m_custom_images;

  std::map<int, FrameInfo> m_measurement_frames;

  std::mutex m_access_mutex;
};

}



#endif /* _SEIMPLEMENTATION_CHECKIMAGES_CHECKIMAGES_H_ */
