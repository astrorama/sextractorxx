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
 * @file SEFramework/Source/SourceGroupWithOnDemandProperties.h
 * @date 08/04/16
 * @author nikoapos
 */

#ifndef _SEFRAMEWORK_SOURCE_SOURCEGROUP_H
#define _SEFRAMEWORK_SOURCE_SOURCEGROUP_H

#include <set>
#include <iterator>
#include <type_traits>

#include "SEFramework/Source/SourceInterface.h"
#include "SEFramework/Source/SourceGroupInterface.h"
#include "SEFramework/Task/TaskProvider.h"
#include "SEFramework/Property/PropertyHolder.h"

namespace SourceXtractor {

/**
 * @class SourceGroupWithOnDemandProperties
 * @brief A SourceGroupInterface implementation which used a TaskProvider to compute missing properties
 *
 */
class SourceGroupWithOnDemandProperties : public SourceGroupInterface {

public:

  explicit SourceGroupWithOnDemandProperties(std::shared_ptr<TaskProvider> task_provider);

  /**
   * @brief Destructor
   */
  virtual ~SourceGroupWithOnDemandProperties() = default;

  iterator begin() override;

  iterator end() override;

  const_iterator cbegin() const override;

  const_iterator cend() const override;

  const_iterator begin() const override;

  const_iterator end() const override;

  void addSource(std::unique_ptr<SourceInterface> source) override;

  iterator removeSource(iterator pos) override;

  void merge(SourceGroupInterface&& other) override;

  unsigned int size() const override;

  void visitProperties(const PropertyVisitor& visitor) override;

  std::unique_ptr<SourceInterface> clone() const override;

  using SourceInterface::getProperty;
  using SourceInterface::setProperty;

protected:

  const Property& getProperty(const PropertyId& property_id) const override;

  void setProperty(std::shared_ptr<Property> property, const PropertyId& property_id) override;

private:

  class EntangledSource;
  std::list<SourceWrapper> m_sources;
  PropertyHolder m_property_holder;
  std::shared_ptr<TaskProvider> m_task_provider;

  void clearGroupProperties();

}; /* End of SourceGroup class */



class SourceGroupWithOnDemandProperties::EntangledSource : public SourceInterface {

public:

  EntangledSource(std::unique_ptr<SourceInterface> source, SourceGroupWithOnDemandProperties& group);

  virtual ~EntangledSource() = default;

  const Property& getProperty(const PropertyId& property_id) const override;

  void setProperty(std::shared_ptr<Property> property, const PropertyId& property_id) override;

  void visitProperties(const PropertyVisitor& visitor) override;

  std::unique_ptr<SourceInterface> clone() const override;

  bool operator<(const EntangledSource& other) const;

private:

  PropertyHolder m_property_holder;
  std::unique_ptr<SourceInterface> m_source;
  SourceGroupWithOnDemandProperties& m_group;

  friend void SourceGroupWithOnDemandProperties::clearGroupProperties();
  friend void SourceGroupWithOnDemandProperties::merge(SourceGroupInterface&&);
  friend std::unique_ptr<SourceInterface> SourceGroupWithOnDemandProperties::clone() const;
};

} /* namespace SourceXtractor */

#endif
