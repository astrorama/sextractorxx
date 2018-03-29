/**
 * @file src/lib/Task/DetectionFramePixelValuesTask.cpp
 * @date 06/16/16
 * @author mschefer
 */
#include <memory>

#include "SEImplementation/Property/PixelCoordinateList.h"
#include "SEFramework/Property/DetectionFrame.h"

#include "SEImplementation/Plugin/DetectionFramePixelValues/DetectionFramePixelValues.h"
#include "SEImplementation/Plugin/DetectionFramePixelValues/DetectionFramePixelValuesTask.h"

namespace SExtractor {

void DetectionFramePixelValuesTask::computeProperties(SourceInterface& source) const {
  auto detection_image = source.getProperty<DetectionFrame>().getFrame()->getSubtractedImage();
  auto filtered_image = source.getProperty<DetectionFrame>().getFrame()->getFilteredImage();
  auto variance_map = source.getProperty<DetectionFrame>().getFrame()->getVarianceMap();

  std::vector<DetectionImage::PixelType> values, filtered_values;
  std::vector<WeightImage::PixelType> variances;
  for (auto pixel_coord : source.getProperty<PixelCoordinateList>().getCoordinateList()) {
    values.push_back(detection_image->getValue(pixel_coord.m_x, pixel_coord.m_y));
    filtered_values.push_back(filtered_image->getValue(pixel_coord.m_x, pixel_coord.m_y));
    variances.push_back(variance_map->getValue(pixel_coord.m_x, pixel_coord.m_y));
  }

  source.setProperty<DetectionFramePixelValues>(std::move(values), std::move(filtered_values), std::move(variances));
}

} // SEImplementation namespace

