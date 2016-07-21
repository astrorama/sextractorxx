/**
 * @file SEImplementation/SegmentationFactory.h
 * @date 06/08/16
 * @author mschefer
 */

#ifndef _SEIMPLEMENTATION_SEGMENTATIONFACTORY_H
#define _SEIMPLEMENTATION_SEGMENTATIONFACTORY_H

#include "SEFramework/Task/TaskRegistry.h"
#include "SEFramework/Configuration/Configurable.h"
#include "SEFramework/Pipeline/Segmentation.h"

namespace SExtractor {

/**
 * @class SegmentationFactory
 * @brief The SegmentationFactory will provide a Segmentation implementation based on the current configuration
 *
 */
class SegmentationFactory : public Configurable {

public:

  /// Destructor
  virtual ~SegmentationFactory() = default;

  /// Constructor
  SegmentationFactory(std::shared_ptr<TaskRegistry> task_registry);

  std::shared_ptr<Segmentation> getSegmentation() const;
  
  // Implementation of the Configurable interface
  void configure(Euclid::Configuration::ConfigManager& manager) override;
  void reportConfigDependencies(Euclid::Configuration::ConfigManager& manager) override;

private:
  
  std::shared_ptr<TaskRegistry> m_task_registry;
  std::shared_ptr<Segmentation> m_segmentation;

}; /* End of SegmentationFactory class */

} /* namespace SExtractor */


#endif
