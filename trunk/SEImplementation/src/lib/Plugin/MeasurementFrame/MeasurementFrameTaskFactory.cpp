/*
 * MeasurementFrameTaskFactory.cpp
 *
 *  Created on: Nov 3, 2016
 *      Author: mschefer
 */

#include "SEImplementation/Configuration/MeasurementConfig.h"
#include "SEImplementation/Plugin/MeasurementFrame/MeasurementFrame.h"
#include "SEImplementation/Plugin/MeasurementFrame/MeasurementFrameTask.h"
#include "SEImplementation/Plugin/MeasurementFrame/MeasurementFrameTaskFactory.h"

namespace SExtractor {

std::shared_ptr<Task> MeasurementFrameTaskFactory::createTask(const PropertyId& property_id) const {
  if (property_id.getTypeId() == PropertyId::create<MeasurementFrame>().getTypeId()) {
    auto instance = property_id.getIndex();

    if (instance < m_measurement_images.size()) {
      return std::make_shared<MeasurementFrameTask>(instance, m_measurement_images[instance]);
    } else if (instance == 0) {
      // By default if no measurement image is provided we use the detection image as the first measurement image
      return std::make_shared<DefaultMeasurementFrameTask>(instance);
    } else {
      return nullptr;
    }
  } else {
    return nullptr;
  }
}

void MeasurementFrameTaskFactory::reportConfigDependencies(Euclid::Configuration::ConfigManager& manager) const {
  manager.registerConfiguration<MeasurementConfig>();
}

void MeasurementFrameTaskFactory::configure(Euclid::Configuration::ConfigManager& manager) {
  m_measurement_images = manager.getConfiguration<MeasurementConfig>().getMeasurementImages();
}

}
