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
 * CheckImagesConfig.h
 *
 *  Created on: May 29, 2017
 *      Author: mschefer
 */

#ifndef _SEIMPLEMENTATION_CONFIGURATION_CHECKIMAGESCONFIG_H_
#define _SEIMPLEMENTATION_CONFIGURATION_CHECKIMAGESCONFIG_H_

#include "Configuration/Configuration.h"
#include "SEFramework/Image/Image.h"

namespace SourceXtractor {

class CheckImagesConfig : public Euclid::Configuration::Configuration {

public:

  explicit CheckImagesConfig(long manager_id);

  virtual ~CheckImagesConfig() = default;

  std::map<std::string, Configuration::OptionDescriptionList> getProgramOptions() override;

  void initialize(const UserValues& args) override;

  const std::string& getModelFittingImageFilename() const {
    return m_model_fitting_filename;
  }

  const std::string& getFittingWindowImageFilename() const {
    return m_fitting_window_filename;
  }

  const std::string& getModelFittingResidualFilename() const {
    return m_model_fitting_residual_filename;
  }

  const std::string& getBackgroundFilename() const {
    return m_background_filename;
  }

  const std::string& getVarianceFilename() const {
    return m_variance_filename;
  }

  const std::string& getMeasurementBackgroundFilename() const {
    return m_measurement_background_filename;
  }

  const std::string& getMeasurementVarianceFilename() const {
    return m_measurement_variance_filename;
  }

  const std::string& getSegmentationFilename() const {
    return m_segmentation_filename;
  }

  const std::string& getPartitionFilename() const {
    return m_partition_filename;
  }

  const std::string& getGroupFilename() const {
    return m_group_filename;
  }

  const std::string& getFilteredFilename() const {
    return m_filtered_filename;
  }

  const std::string& getThresholdedFilename() const {
    return m_thresholded_filename;
  }

  const std::string& getSnrFilename() const {
    return m_snr_filename;
  }

  const std::string& getAutoApertureFilename() const {
    return m_auto_aperture_filename;
  }

  const std::string& getApertureFilename() const {
    return m_aperture_filename;
  }

  const std::string& getMoffatFilename() const {
    return m_moffat_filename;
  }

  const std::string& getPsfFilename() const {
    return m_psf_filename;
  }

  const std::string& getMLDetectionFilename() const {
    return m_ml_detection_filename;
  }

private:

  std::string m_model_fitting_filename;
  std::string m_fitting_window_filename;
  std::string m_model_fitting_residual_filename;
  std::string m_background_filename;
  std::string m_variance_filename;
  std::string m_measurement_background_filename;
  std::string m_measurement_variance_filename;
  std::string m_segmentation_filename;
  std::string m_partition_filename;
  std::string m_group_filename;
  std::string m_filtered_filename;
  std::string m_thresholded_filename;
  std::string m_snr_filename;
  std::string m_auto_aperture_filename;
  std::string m_aperture_filename;
  std::string m_moffat_filename;
  std::string m_psf_filename;
  std::string m_ml_detection_filename;
};

}



#endif /* _SEIMPLEMENTATION_CONFIGURATION_CheckImagesSCONFIG_H_ */
