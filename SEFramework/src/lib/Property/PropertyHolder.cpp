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
 * @file src/lib/Property/ObjectWithProperties.cpp
 * @date 05/03/16
 * @author mschefer
 */

#include "SEFramework/Property/PropertyHolder.h"

#include "SEFramework/Property/PropertyNotFoundException.h"

namespace SourceXtractor {

const Property& PropertyHolder::getProperty(const PropertyId& property_id) const {
  auto iter = m_properties.find(property_id);
  if (iter != m_properties.end()) {
    // Returns the property if it is found
    return *iter->second;
  } else {
    // If we don't have that property throws an exception
    throw PropertyNotFoundException(property_id);
  }
}

void PropertyHolder::setProperty(std::shared_ptr<Property> property, const PropertyId& property_id) {
  m_properties[property_id] = std::move(property);
}

bool PropertyHolder::isPropertySet(const PropertyId& property_id) const {
  return m_properties.find(property_id) != m_properties.end();
}

void PropertyHolder::clear() {
  m_properties.clear();
}

void PropertyHolder::update(const SourceXtractor::PropertyHolder& other) {
  std::copy(other.m_properties.begin(), other.m_properties.end(), std::inserter(m_properties, m_properties.begin()));
}

} // SEFramework namespace
