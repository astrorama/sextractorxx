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
/*
 * @file EntangledSource.cpp
 * @author nikoapos
 */

#include "SEFramework/Source/SourceGroupWithOnDemandProperties.h"
#include "SEFramework/Task/GroupTask.h"

namespace SourceXtractor {

SourceGroupWithOnDemandProperties::EntangledSource::EntangledSource(std::unique_ptr<SourceInterface> source, SourceGroupWithOnDemandProperties& group)
        : m_source(std::move(source)), m_group(group) {
  // Normally, it should not be possible that the given source is of type
  // EntangledSource, because the entangled sources of a group can only be
  // accessed via the iterator as references. Nevertheless, to be sure that
  // future changes will not change the behavior, we do a check to the given
  // source and if it is an EntangledSource we use its encapsulated source instead.
  auto entangled_ptr = dynamic_cast<EntangledSource*>(m_source.get());
  if (entangled_ptr != nullptr) {
    m_source = std::move(entangled_ptr->m_source);
  }
}

const Property& SourceGroupWithOnDemandProperties::EntangledSource::getProperty(const PropertyId& property_id) const {

  // If we already have the property stored in this object, returns it
  if (m_property_holder.isPropertySet(property_id)) {
    return m_property_holder.getProperty(property_id);
  }

  // If the property is already stored in the group, we return it
  if (m_group.m_property_holder.isPropertySet(property_id)) {
    return m_group.getProperty(property_id);
  }

  try {
    // Try to get the the property from the encapsulated Source
    // if it cannot provide it, this will throw a PropertyNotFoundException
    return m_source->getProperty(property_id);
  } catch (PropertyNotFoundException& e) {
    // Getting this exception means the property must be computed at the group level

    // Get the group task
    auto group_task = m_group.m_task_provider->getTask<GroupTask>(property_id);
    if (!group_task) {
      // No task is available to make that property
      throw PropertyNotFoundException(property_id);
    }

  // Use the task to make the property
    group_task->computeProperties(m_group);

    // The property should now be available either in this object or in the group object
    if (m_property_holder.isPropertySet(property_id)) {
      return m_property_holder.getProperty(property_id);
    } else {
      return m_group.m_property_holder.getProperty(property_id);
    }
  }

} // end of getProperty()

void SourceGroupWithOnDemandProperties::EntangledSource::setProperty(std::shared_ptr<Property> property,
                                                                     const PropertyId&         property_id) {
  m_property_holder.setProperty(std::move(property), property_id);
}

std::unique_ptr<SourceInterface> SourceGroupWithOnDemandProperties::EntangledSource::clone() const {
  throw Elements::Exception("Can not clone an entangled source");
}

bool SourceGroupWithOnDemandProperties::EntangledSource::operator<(const EntangledSource& other) const {
  return this->m_source < other.m_source;
}

} // SourceXtractor namespace
