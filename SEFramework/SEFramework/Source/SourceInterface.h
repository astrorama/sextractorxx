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
 * @file SEFramework/Source/SourceInterface.h
 * @date 05/04/16
 * @author mschefer
 */

#ifndef _SEFRAMEWORK_SOURCE_SOURCEINTERFACE_H
#define _SEFRAMEWORK_SOURCE_SOURCEINTERFACE_H

#include <memory>
#include <type_traits>

#include "SEFramework/Property/PropertyId.h"
#include "SEFramework/Property/PropertyNotFoundException.h"
#include "SEFramework/Property/Property.h"

namespace SourceXtractor {

/**
 * @class SourceInterface
 * @brief The SourceInterface is an abstract "source" that has properties attached to it.
 *
 * @details
 * Properties are accessed through the templated method getProperty<>()/setProperty<>().
 *
 * Actual implementation in sub-classed is done by overriding the virtual getProperty()/setProperty()
 *
 */

class SourceInterface {

public:
  using PropertyVisitor = std::function<void(const PropertyId&, const std::shared_ptr<Property>&)>;

  /**
   * @brief Destructor
   */
  virtual ~SourceInterface() = default;

  /// Convenience template method to call getProperty() with a more user-friendly syntax
  template<typename PropertyType>
  const PropertyType& getProperty(unsigned int index = 0) const {
    static_assert(std::is_base_of<Property, PropertyType>::value, "PropertyType must inherit from SourceXtractor::Property");
    return dynamic_cast<const PropertyType&>(getProperty(PropertyId::create<PropertyType>(index)));
  }

  /// Convenience template method to call setProperty() with a more user-friendly syntax
  template<typename PropertyType, typename ... Args>
  void setIndexedProperty(std::size_t index, Args... args) {
    static_assert(std::is_base_of<Property, PropertyType>::value, "PropertyType must inherit from SourceXtractor::Property");
    static_assert(std::is_constructible<PropertyType, Args...>::value, "PropertyType must be constructible from args");
    setProperty(std::unique_ptr<PropertyType>{new PropertyType(std::forward<Args>(args)...)},
                PropertyId::create<PropertyType>(index));
  }
  
  template<typename PropertyType, typename ... Args>
  void setProperty(Args... args) {
    setIndexedProperty<PropertyType>(0, std::forward<Args>(args)...);
  }

  /// Call PropertyVisitor once per *set* property. On demand properties are ignored if not yet computed.
  /// Concrete implementations must call the visitor from lower to higher preference (i.e. group properties before
  /// individual source properties), so in case of conflict the higher priority property can take precedence.
  virtual void visitProperties(const PropertyVisitor&) = 0;

  /// Returns a reference to the requested property. The property may be computed if needed
  /// Throws a PropertyNotFoundException if the property cannot be provided.
  virtual const Property& getProperty(const PropertyId& property_id) const = 0;
  virtual void setProperty(std::shared_ptr<Property> property, const PropertyId& property_id) = 0;

  virtual std::unique_ptr<SourceInterface> clone() const = 0;

}; /* End of SourceInterface class */

} /* namespace SourceXtractor */


#endif
