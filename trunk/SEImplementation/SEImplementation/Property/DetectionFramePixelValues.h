/**
 * @file SEImplementation/Property/DetectionFramePixelValues.h
 * @date 05/27/16
 * @author mschefer
 */

#ifndef _SEIMPLEMENTATION_PROPERTY_DETECTIONFRAMEPIXELVALUES_H
#define _SEIMPLEMENTATION_PROPERTY_DETECTIONFRAMEPIXELVALUES_H

#include "SEFramework/Property/Property.h"
#include "SEFramework/Source/Source.h"
#include "SEFramework/Task/SourceTask.h"
#include "SEFramework/Task/TaskFactory.h"

namespace SEFramework {
  class Image;
}

namespace SEImplementation {


using SEFramework::Image;
using SEFramework::Property;
using SEFramework::PropertyId;
using SEFramework::Source;
using SEFramework::SourceTask;
using SEFramework::Task;
using SEFramework::TaskFactory;

/**
 * @class DetectionFramePixelValues
 * @brief
 *
 */
class DetectionFramePixelValues : public Property {

public:

  /**
   * @brief Destructor
   */
  virtual ~DetectionFramePixelValues() = default;

  DetectionFramePixelValues(std::vector<double> values) : m_values(std::move(values)) {}

  const std::vector<double>& getValues() const {
    return m_values;
  }

private:

  std::vector<double> m_values;

}; /* End of DetectionFramePixelValues class */

/**
 * @class PixelCentroid
 * @brief
 *
 */
class DetectionFramePixelValuesTask : public SourceTask {

public:

  /**
   * @brief Destructor
   */
  virtual ~DetectionFramePixelValuesTask() = default;

  DetectionFramePixelValuesTask(std::shared_ptr<Image> image);

  virtual void computeProperties(Source& source) const override;

private:
  std::shared_ptr<Image> m_image;

}; /* End of DetectionFramePixelValuesTask class */

class DetectionFramePixelValuesTaskFactory : public TaskFactory {
public:
  DetectionFramePixelValuesTaskFactory();

  /**
   * @brief Destructor
   */
  virtual ~DetectionFramePixelValuesTaskFactory() = default;

  virtual std::shared_ptr<Task> getTask(PropertyId property_id) override;
  virtual const std::vector<PropertyId> getProducedProperties() override;

private:
  std::shared_ptr<DetectionFramePixelValuesTask> m_detection_frame_pixel_values_task;
};

} /* namespace SEImplementation */


#endif
