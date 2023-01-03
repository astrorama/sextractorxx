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
 * @file SEFramework/Property/ObjectWithProperties.h
 * @date 05/03/16
 * @author mschefer
 */

#ifndef _SEFRAMEWORK_PROPERTY_PROPERTYHOLDER_H
#define _SEFRAMEWORK_PROPERTY_PROPERTYHOLDER_H

#include <memory>
#include <unordered_map>

#include "SEFramework/Property/PropertyId.h"
#include "SEFramework/Property/Property.h"

namespace SourceXtractor {


/**
 * @class PropertyHolder
 * @brief A class providing a simple implementation of a container of properties.
 *
 * @details This class is used to provide a common implementation for objects that have properties
 *
 */

class PropertyHolder {

public:

  using const_iterator = std::unordered_map<PropertyId, std::shared_ptr<Property>>::const_iterator;

  /// Destructor
  virtual ~PropertyHolder() = default;

  // removes copy/move constructors and assignment operators
  PropertyHolder(const PropertyHolder&) = delete;
  PropertyHolder& operator=(const PropertyHolder&) = delete;
  PropertyHolder(PropertyHolder&&) = delete;
  PropertyHolder& operator=(PropertyHolder&&) = delete;

  /// Constructors
  PropertyHolder() {}

  /// Returns a reference to a Property if it is set, if not throws a PropertyNotFoundException
  const Property& getProperty(const PropertyId& property_id) const;

  /// Sets a property, overwriting it if necessary
  void setProperty(std::shared_ptr<Property> property, const PropertyId& property_id);

  /// Returns true if the property is set
  bool isPropertySet(const PropertyId& property_id) const;

  void clear();

  void update(const PropertyHolder& other);

  const_iterator begin() const {
    return m_properties.begin();
  }

  const_iterator end() const {
    return m_properties.end();
  }

private:

  std::unordered_map<PropertyId, std::shared_ptr<Property>> m_properties;

}; /* End of ObjectWithProperties class */

} /* namespace SourceXtractor */


#endif
