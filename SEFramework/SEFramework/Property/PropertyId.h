/*
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
 * @file SEFramework/Property/PropertyId.h
 * @date 05/17/16
 * @author mschefer
 */

#ifndef _SEFRAMEWORK_PROPERTY_PROPERTYID_H
#define _SEFRAMEWORK_PROPERTY_PROPERTYID_H

#include <typeindex>
#include <string>
#include <functional>

#include <boost/functional/hash.hpp>

namespace SourceXtractor {

/**
 * @class PropertyId
 * @brief Identifier used to set and retrieve properties.
 *
 */

class PropertyId {
public:
  PropertyId(std::type_index type_id, unsigned int index) : m_type_id(type_id), m_index(index) {}

  /// Templated factory method used to create a PropertyId based on the type of a property.
  /// An optional index parameter is used to make the distinction between several properties of the same type.
  template<typename T>
  static PropertyId create(unsigned int index = 0) {
    return PropertyId(typeid(T), index);
  }

  /// Equality operator is needed to be use PropertyId as key in unordered_map
  bool operator==(PropertyId other) const {
    // A PropertyId is equal to another if both their type_id and index are the same
    return m_type_id == other.m_type_id && m_index == other.m_index;
  }

  /// Less than operator needed to use PropertyId as key in a std::map
  bool operator<(PropertyId other) const {
    // if both type_ids are equal, use index as the tie breaker
    if (m_type_id == other.m_type_id) {
      return m_index < other.m_index;
    }

    // order PropertyIds by their type_ids
    return m_type_id < other.m_type_id;
  }

  std::type_index getTypeId() const {
    return m_type_id;
  }

  unsigned int getIndex() const {
    return m_index;
  }

  std::string getString() const;

private:

  std::type_index m_type_id;
  unsigned int m_index;
  friend struct std::hash<SourceXtractor::PropertyId>;
};

}

namespace std {

// defines a hash for PropertyId, this is to be able to use PropertyId as a key in a std::unordered_map

template <>
struct hash<SourceXtractor::PropertyId>
{
  std::size_t operator()(const SourceXtractor::PropertyId& id) const {
    std::size_t h = 0;
    boost::hash_combine(h, id.m_type_id);
    boost::hash_combine(h, id.m_index);
    return h;
  }
};

}

#endif
