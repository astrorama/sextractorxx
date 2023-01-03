/**
 * Copyright © 2019-2022 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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
/**
 * @file SEFramework/Source/SourceWithOnDemandProperties.h
 * @date 05/04/16
 * @author mschefer
 */

#ifndef _SEFRAMEWORK_SOURCE_SOURCE_H
#define _SEFRAMEWORK_SOURCE_SOURCE_H

#include <vector>

#include "SEFramework/Property/PropertyHolder.h"
#include "SEFramework/Source/SourceInterface.h"

namespace SourceXtractor {

class TaskProvider;

/**
 * @class SourceWithOnDemandProperties
 *
 * @details Provides getProperty() and setProperty() implementations which use
 * a TaskProvider to compute the non already available properties on demand.
 *
 */

class SourceWithOnDemandProperties : public SourceInterface {

public:

  /**
   * @brief Destructor
   */
  virtual ~SourceWithOnDemandProperties() = default;

  // removes copy/move constructors and assignment operators
  SourceWithOnDemandProperties& operator=(const SourceWithOnDemandProperties&) = delete;
  SourceWithOnDemandProperties(SourceWithOnDemandProperties&&) = delete;
  SourceWithOnDemandProperties& operator=(SourceWithOnDemandProperties&&) = delete;

  /// Constructor
  explicit SourceWithOnDemandProperties(std::shared_ptr<const TaskProvider> task_provider);

  void visitProperties(const PropertyVisitor& visitor) override;

  // Note : Because the get/setProperty() methods of the SourceInterface are
  // templated, the overrides of the non-templated versions will hide them. For
  // this reason it is necessary to re-introduce the templated methods, which is
  // done by the using statements below.
  using SourceInterface::getProperty;
  using SourceInterface::setProperty;

  std::unique_ptr<SourceInterface> clone() const override;
  
protected:
  
  // Implementation of SourceInterface
  const Property& getProperty(const PropertyId& property_id) const override;
  void setProperty(std::shared_ptr<Property> property, const PropertyId& property_id) override;

private:
  SourceWithOnDemandProperties(const SourceWithOnDemandProperties& other);

  std::shared_ptr<const TaskProvider> m_task_provider;
  PropertyHolder m_property_holder;
}; /* End of Source class */

} /* namespace SourceXtractor */

#endif
