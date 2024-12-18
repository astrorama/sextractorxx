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
 * CheckImages.cpp
 *
 *  Created on: May 30, 2017
 *      Author: mschefer
 */

#include "SEFramework/Image/ProcessedImage.h"
#include "SEFramework/FITS/FitsWriter.h"
#include "SEImplementation/Configuration/DetectionImageConfig.h"
#include "SEImplementation/Configuration/MeasurementImageConfig.h"
#include "SEImplementation/Configuration/MeasurementFrameConfig.h"
#include "SEImplementation/Configuration/CheckImagesConfig.h"

#include "SEImplementation/CheckImages/CheckImages.h"

namespace SourceXtractor {

namespace {

std::string addNumberToFilename(boost::filesystem::path original_filename, size_t number, bool add_number) {
  if (add_number) {
    auto filename = original_filename.stem();
    filename += "_" + std::to_string(number);
    filename += original_filename.extension();

    filename = original_filename.parent_path() / filename;

    return filename.native();
  } else {
    return original_filename.native();
  }
}

}

std::unique_ptr<CheckImages> CheckImages::m_instance;

CheckImages::CheckImages() {
}

void CheckImages::reportConfigDependencies(Euclid::Configuration::ConfigManager &manager) const {
  manager.registerConfiguration<CheckImagesConfig>();
  manager.registerConfiguration<DetectionImageConfig>();
  manager.registerConfiguration<MeasurementImageConfig>();
  manager.registerConfiguration<MeasurementFrameConfig>();
}

std::shared_ptr<WriteableImage<SeFloat>> CheckImages::getWriteableCheckImage(std::string id, int width, int height) {
  if (m_custom_images.count(id) != 0) {
    auto image = std::dynamic_pointer_cast<WriteableImage<SeFloat>>(std::get<0>(m_custom_images[id]));
    if (image != nullptr) {
      return image;
    }
  }

  auto image = FitsWriter::newImage<SeFloat>(id + ".fits",
      width, height);
  m_custom_images[id] = std::make_tuple(image, false);

  return image;
}

void CheckImages::setCustomCheckImage(std::string id, std::shared_ptr<Image<SeFloat>> image) {
  m_custom_images[id] = std::make_tuple(image, true);
}

void CheckImages::configure(Euclid::Configuration::ConfigManager& manager) {
  auto& config = manager.getConfiguration<CheckImagesConfig>();

  m_model_fitting_image_filename = config.getModelFittingImageFilename();
  m_fitting_window_image_filename = config.getFittingWindowImageFilename();
  m_residual_filename = config.getModelFittingResidualFilename();
  m_background_filename = config.getBackgroundFilename();
  m_variance_filename = config.getVarianceFilename();
  m_segmentation_filename = config.getSegmentationFilename();
  m_partition_filename = config.getPartitionFilename();
  m_group_filename = config.getGroupFilename();
  m_filtered_filename = config.getFilteredFilename();
  m_thresholded_filename = config.getThresholdedFilename();
  m_snr_filename = config.getSnrFilename();
  m_auto_aperture_filename = config.getAutoApertureFilename();
  m_aperture_filename = config.getApertureFilename();
  m_moffat_filename = config.getMoffatFilename();
  m_psf_filename = config.getPsfFilename();
  m_ml_detection_filename = config.getMLDetectionFilename();

  m_measurement_background_filename = config.getMeasurementBackgroundFilename();
  m_measurement_variance_filename = config.getMeasurementVarianceFilename();

  size_t detection_images_nb = manager.getConfiguration<DetectionImageConfig>().getExtensionsNb();

  m_check_image_ml_detection.resize(detection_images_nb);

  for (size_t i = 0; i < detection_images_nb; i++) {
    auto detection_image = manager.getConfiguration<DetectionImageConfig>().getDetectionImage();
    auto coordinate_system = manager.getConfiguration<DetectionImageConfig>().getCoordinateSystem();

    m_detection_images.emplace_back(detection_image);
    m_coordinate_systems.emplace_back(coordinate_system);

    if (m_segmentation_filename != "") {
      m_segmentation_images.emplace_back(FitsWriter::newImage<int>(
          addNumberToFilename(m_segmentation_filename, i, detection_images_nb>1),
          detection_image->getWidth(), detection_image->getHeight(), coordinate_system));
    }

    if (m_partition_filename != "") {
      m_partition_images.emplace_back(FitsWriter::newImage<int>(
          addNumberToFilename(m_partition_filename, i, detection_images_nb>1),
          detection_image->getWidth(), detection_image->getHeight(), coordinate_system));
    }

    if (m_group_filename != "") {
      m_group_images.emplace_back(FitsWriter::newImage<int>(
          addNumberToFilename(m_group_filename, i, detection_images_nb>1),
          detection_image->getWidth(), detection_image->getHeight(), coordinate_system));
    }

    if (m_auto_aperture_filename != "") {
      m_auto_aperture_images.emplace_back(FitsWriter::newImage<int>(
          addNumberToFilename(m_auto_aperture_filename, i, detection_images_nb>1),
          detection_image->getWidth(), detection_image->getHeight(), coordinate_system));
    }

    if (m_aperture_filename != "") {
      m_aperture_images.emplace_back(FitsWriter::newImage<int>(
          addNumberToFilename(m_aperture_filename, i, detection_images_nb>1),
          detection_image->getWidth(), detection_image->getHeight(), coordinate_system));
    }

    if (m_moffat_filename != "") {
      m_moffat_images.emplace_back(FitsWriter::newImage<SeFloat>(
          addNumberToFilename(m_moffat_filename, i, detection_images_nb>1),
          detection_image->getWidth(), detection_image->getHeight(), coordinate_system));
    }
  }

  // Measurement images
  const auto& measurement_images_info = manager.getConfiguration<MeasurementImageConfig>().getImageInfos();
  const auto& frames = manager.getConfiguration<MeasurementFrameConfig>().getFrames();
  for (auto& info : measurement_images_info) {
    std::stringstream label;
    label << boost::filesystem::path(info.m_path).stem().string() << "_" << info.m_image_hdu;
    if (info.m_is_data_cube) {
      label << "_" << info.m_image_layer;
    }

    m_measurement_frames[info.m_id] = FrameInfo {
      label.str(),
      info.m_measurement_image->getWidth(),
      info.m_measurement_image->getHeight(),
      info.m_coordinate_system,
      frames.at(info.m_id)->getImage(LayerSubtractedImage)
    };
  }
}

std::shared_ptr<WriteableImage<int>> CheckImages::getMeasurementAutoApertureImage(unsigned int frame_number) {
  std::lock_guard<std::mutex> lock{m_access_mutex};

  if (m_auto_aperture_filename.empty()) {
    return nullptr;
  }

  auto i = m_measurement_auto_aperture_images.find(frame_number);
  if (i == m_measurement_auto_aperture_images.end()) {
    auto& frame_info = m_measurement_frames.at(frame_number);
    auto filename = m_auto_aperture_filename.stem();
    filename += "_" + frame_info.m_label;
    filename += m_auto_aperture_filename.extension();
    auto frame_filename = m_auto_aperture_filename.parent_path() / filename;
    i = m_measurement_auto_aperture_images.emplace(
      std::make_pair(
        frame_number,
        FitsWriter::newImage<int>(
          frame_filename.native(),
          frame_info.m_width,
          frame_info.m_height,
          frame_info.m_coordinate_system
        ))).first;
  }
  return LockedWriteableImage<int>::create(i->second);
}

std::shared_ptr<WriteableImage<int>> CheckImages::getMeasurementApertureImage(unsigned int frame_number) {
  std::lock_guard<std::mutex> lock{m_access_mutex};

  if (m_aperture_filename.empty()) {
    return nullptr;
  }

  auto i = m_measurement_aperture_images.find(frame_number);
  if (i == m_measurement_aperture_images.end()) {
    auto& frame_info = m_measurement_frames.at(frame_number);
    auto filename = m_aperture_filename.stem();
    filename += "_" + frame_info.m_label;
    filename += m_aperture_filename.extension();
    auto frame_filename = m_aperture_filename.parent_path() / filename;
    i = m_measurement_aperture_images.emplace(
      std::make_pair(
        frame_number,
        FitsWriter::newImage<int>(
          frame_filename.native(),
          frame_info.m_width,
          frame_info.m_height,
          frame_info.m_coordinate_system
        ))).first;
  }
  return LockedWriteableImage<int>::create(i->second);
}

std::shared_ptr<WriteableImage<MeasurementImage::PixelType>>
CheckImages::getModelFittingImage(unsigned int frame_number) {
  std::lock_guard<std::mutex> lock{m_access_mutex};

  if (m_model_fitting_image_filename.empty() && m_residual_filename.empty()) {
    return nullptr;
  }

  auto i = m_check_image_model_fitting.find(frame_number);
  if (i == m_check_image_model_fitting.end()) {
    auto& frame_info = m_measurement_frames.at(frame_number);
    std::shared_ptr<WriteableImage<MeasurementImage::PixelType>> writeable_image;

    if (m_model_fitting_image_filename.empty()) {
      writeable_image = FitsWriter::newTemporaryImage<DetectionImage::PixelType>(
        "sourcextractor_check_model_%%%%%%.fits",
        frame_info.m_width, frame_info.m_height
      );
    } else {
      auto filename = m_model_fitting_image_filename.stem();
      filename += "_" + frame_info.m_label;
      filename += m_model_fitting_image_filename.extension();
      auto frame_filename = m_model_fitting_image_filename.parent_path() / filename;
      writeable_image = FitsWriter::newImage<MeasurementImage::PixelType>(
        frame_filename.native(),
        frame_info.m_width,
        frame_info.m_height,
        frame_info.m_coordinate_system
      );
    }
    i = m_check_image_model_fitting.emplace(std::make_pair(frame_number, writeable_image)).first;
  }
  return LockedWriteableImage<MeasurementImage::PixelType>::create(i->second);
}

std::shared_ptr<WriteableImage<int>>
CheckImages::getFittingWindowImage(unsigned int frame_number) {
  std::lock_guard<std::mutex> lock{m_access_mutex};

  if (m_fitting_window_image_filename.empty()) {
    return nullptr;
  }

  auto i = m_check_image_fitting_window.find(frame_number);
  if (i == m_check_image_fitting_window.end()) {
    auto& frame_info = m_measurement_frames.at(frame_number);
    auto filename = m_fitting_window_image_filename.stem();
    filename += "_" + frame_info.m_label;
    filename += m_fitting_window_image_filename.extension();
    auto frame_filename = m_fitting_window_image_filename.parent_path() / filename;
    auto writeable_image = FitsWriter::newImage<int>(
      frame_filename.native(),
      frame_info.m_width,
      frame_info.m_height,
      frame_info.m_coordinate_system
    );
    i = m_check_image_fitting_window.emplace(std::make_pair(frame_number, writeable_image)).first;
  }
  return LockedWriteableImage<int>::create(i->second);
}


std::shared_ptr<WriteableImage<MeasurementImage::PixelType>> CheckImages::getPsfImage(unsigned int frame_number) {
  std::lock_guard<std::mutex> lock{m_access_mutex};

  if (m_psf_filename.empty()) {
    return nullptr;
  }

  auto i = m_check_image_psf.find(frame_number);
  if (i == m_check_image_psf.end()) {
    auto& frame_info = m_measurement_frames.at(frame_number);
    auto filename = m_psf_filename.stem();
    filename += "_" + frame_info.m_label;
    filename += m_psf_filename.extension();
    auto frame_filename = m_psf_filename.parent_path() / filename;
    i = m_check_image_psf.emplace(
      std::make_pair(
        frame_number,
        FitsWriter::newImage<MeasurementImage::PixelType>(
          frame_filename.native(),
          frame_info.m_width,
          frame_info.m_height,
          frame_info.m_coordinate_system
        ))).first;
  }
  return LockedWriteableImage<MeasurementImage::PixelType>::create(i->second);
}

std::shared_ptr<WriteableImage<MeasurementImage::PixelType>>
    CheckImages::getMLDetectionImage(unsigned int plane_number, size_t index) {
  std::lock_guard<std::mutex> lock{m_access_mutex};

  if (m_ml_detection_filename.empty()) {
    return nullptr;
  }

  auto i = m_check_image_ml_detection.at(index).find(plane_number);
  if (i == m_check_image_ml_detection.at(index).end()) {
    auto filename = m_ml_detection_filename.stem();
    filename += "_" + std::to_string(plane_number);
    filename += m_ml_detection_filename.extension();
    auto frame_filename = m_ml_detection_filename.parent_path() / filename;
    i = m_check_image_ml_detection.at(index).emplace(
      std::make_pair(
        plane_number,
        FitsWriter::newImage<MeasurementImage::PixelType>(
          frame_filename.native(),
          m_detection_images.at(index)->getWidth(),
          m_detection_images.at(index)->getHeight(),
          m_coordinate_systems.at(index)
        ))).first;
  }
  return LockedWriteableImage<MeasurementImage::PixelType>::create(i->second);
}

void CheckImages::saveImages() {
  std::lock_guard<std::mutex> lock(m_access_mutex);

  auto detection_images_nb = m_coordinate_systems.size();
  for (size_t i = 0; i < detection_images_nb; i++) {
    // if possible, save the background image
    if (i < m_background_images.size() && m_background_images.at(i) != nullptr && m_background_filename != "") {
      FitsWriter::writeFile(*m_background_images.at(i),
          addNumberToFilename(m_background_filename, i, detection_images_nb>1), m_coordinate_systems.at(i));
    }

    // if possible, save the variance image
    if (i < m_variance_images.size() && m_variance_images.at(i) != nullptr && m_variance_filename != "") {
      FitsWriter::writeFile(*m_variance_images.at(i),
          addNumberToFilename(m_variance_filename, i, detection_images_nb>1), m_coordinate_systems.at(i));
    }

    // if possible, save the filtered image
    if (i < m_filtered_images.size() && m_filtered_images.at(i) != nullptr && m_filtered_filename != "") {
      FitsWriter::writeFile(*m_filtered_images.at(i),
          addNumberToFilename(m_filtered_filename, i, detection_images_nb>1), m_coordinate_systems.at(i));
    }

    // if possible, save the thresholded image
    if (i < m_thresholded_images.size() && m_thresholded_images.at(i) != nullptr && m_thresholded_filename != "") {
      FitsWriter::writeFile(*m_thresholded_images.at(i),
          addNumberToFilename(m_thresholded_filename, i, detection_images_nb>1), m_coordinate_systems.at(i));
    }

    // if possible, save the SNR image
    if (i < m_snr_images.size() && m_snr_images.at(i) != nullptr && m_snr_filename != "") {
      FitsWriter::writeFile(*m_snr_images.at(i),
          addNumberToFilename(m_snr_filename, i, detection_images_nb>1), m_coordinate_systems.at(i));
    }
  }

  // if possible, save the measurement background images
  if (m_measurement_background_filename != "") {
    for (auto &ci : m_measurement_background_images) {
      auto& frame_info = m_measurement_frames.at(ci.first);

      auto background_image = ci.second;
      auto filename = m_measurement_background_filename.stem();
      filename += "_" + frame_info.m_label;
      filename += m_measurement_background_filename.extension();
      auto frame_filename = m_measurement_background_filename.parent_path() / filename;
      FitsWriter::writeFile(*background_image, frame_filename.native(), frame_info.m_coordinate_system);
    }
  }

  // if possible, save the measurement variance images
  if (m_measurement_variance_filename != "") {
    for (auto &ci : m_measurement_variance_images) {
      auto& frame_info = m_measurement_frames.at(ci.first);

      auto variance_image = ci.second;
      auto filename = m_measurement_variance_filename.stem();
      filename += "_" + frame_info.m_label;
      filename += m_measurement_variance_filename.extension();
      auto frame_filename = m_measurement_variance_filename.parent_path() / filename;
      FitsWriter::writeFile(*variance_image, frame_filename.native(), frame_info.m_coordinate_system);
    }
  }

  // if possible, create and save the residual images
  if (m_residual_filename != "") {
    for (auto &ci : m_check_image_model_fitting) {
      auto& frame_info = m_measurement_frames.at(ci.first);

      auto residual_image = SubtractImage<SeFloat>::create(frame_info.m_subtracted_image, ci.second);
      auto filename = m_residual_filename.stem();
      filename += "_" + frame_info.m_label;
      filename += m_residual_filename.extension();

      auto frame_filename = m_residual_filename.parent_path() / filename;
      FitsWriter::writeFile(*residual_image, frame_filename.native(), frame_info.m_coordinate_system);
    }
  }

  for (auto const& entry : m_custom_images) {
    if (std::get<1>(entry.second)) {
      auto filename = entry.first;
      if (!filename.has_extension()) {
        filename += ".fits";
      }
      FitsWriter::writeFile(*std::get<0>(entry.second), filename.native());
    }
  }
}

}
