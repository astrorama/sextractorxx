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
 * @file OutputRegistry.h
 * @author nikoapos
 */

#ifndef _SEFRAMEWORK_OUTPUTREGISTRY_H
#define _SEFRAMEWORK_OUTPUTREGISTRY_H

#include <functional>
#include <vector>
#include <string>
#include <map>
#include <typeindex>

#include "Table/Row.h"
#include "SEFramework/Source/SourceInterface.h"

namespace SourceXtractor {

class OutputRegistry {
public:
  template <typename PropertyType, typename OutType>
  using ColumnConverter = std::function<OutType(const PropertyType&)>;

  using SourceToRowConverter = std::function<Euclid::Table::Row(const SourceInterface&)>;

  class ColumnFromSource {
  public:
    template <typename PropertyType, typename OutType>
    explicit ColumnFromSource(ColumnConverter<PropertyType, OutType> converter) {
      m_convert_func = [converter](const SourceInterface& source, std::size_t i) {
        return converter(source.getProperty<PropertyType>(i));
      };
    }
    Euclid::Table::Row::cell_type operator()(const SourceInterface& source) const {
      return m_convert_func(source, index);
    }
    std::size_t index = 0;

  private:
    std::function<Euclid::Table::Row::cell_type(const SourceInterface&, std::size_t index)> m_convert_func;
  };

  struct ColInfo {
    std::string unit;
    std::string description;
  };

public:
  template <typename PropertyType, typename OutType>
  void registerColumnConverter(std::string column_name, ColumnConverter<PropertyType, OutType> converter,
                               std::string column_unit="", std::string column_description="") {
    m_property_to_names_map[typeid(PropertyType)].emplace_back(column_name);
    m_name_to_property_map.emplace(column_name, typeid(PropertyType));
    std::type_index conv_out_type = typeid(OutType);
    ColumnFromSource conv_func {converter};
    m_name_to_converter_map.emplace(column_name,
                                    std::pair<std::type_index, ColumnFromSource>(conv_out_type, conv_func));
    m_name_to_col_info_map.emplace(column_name, ColInfo{column_unit, column_description});
  }

  std::type_index getPropertyForColumn(const std::string& column_name) const {
    return m_name_to_property_map.at(column_name);
  }

  const std::pair<std::type_index, ColumnFromSource>& getColumnConverter(const std::string& column_name) const {
    return m_name_to_converter_map.at(column_name);
  }

  /**
   * When there are multiple instances of a given property, generate one column output with the given suffix for each
   * instance
   * @tparam PropertyType
   * @param instance_names
   */
  template <typename PropertyType>
  void registerPropertyInstances(const std::vector<std::pair<std::string, unsigned int>>& instance_names) {
    std::vector<std::string> new_names {};
    for (auto& current_name : m_property_to_names_map[typeid(PropertyType)]) {
      // Get the current converter
      auto converter = m_name_to_converter_map.at(current_name);
      auto col_info = m_name_to_col_info_map.at(current_name);
      // Remove the old converter
      // Do it *before*, because the new name may be the same!
      m_name_to_converter_map.erase(current_name);
      m_name_to_col_info_map.erase(current_name);

      // Add the new ones
      for (auto instance : instance_names) {
        // Make a copy of the converter and set the index
        auto new_converter = converter;
        new_converter.second.index = instance.second;
        // Register the new converter with the new name
        auto& postfix = instance.first;
        auto new_name = current_name + "_" + postfix;
        m_name_to_converter_map.emplace(new_name, new_converter);
        m_name_to_col_info_map.emplace(new_name, col_info);
        new_names.push_back(new_name);
      }
    }
    // Replace all the old names with the new ones
    m_property_to_names_map[typeid(PropertyType)] = new_names;
  }

  /**
   * When there are multiple instances of a given property, generate one column output with the given name for each
   * instance replacing an existing registered name
   * @tparam PropertyType
   * @param current_name
   * @param instance_names
   */
  template <typename PropertyType>
  void registerPropertyInstances(const std::string &current_name, const std::vector<std::pair<std::string, unsigned int>>& instance_names) {
    std::vector<std::string> new_names {};
    // Get the current converter
    auto converter = m_name_to_converter_map.at(current_name);
    auto col_info = m_name_to_col_info_map.at(current_name);
    // Remove the old converter
    // Do it *before*, because the new name may be the same!
    m_name_to_converter_map.erase(current_name);
    m_name_to_col_info_map.erase(current_name);

    // Add the new ones
    for (auto instance : instance_names) {
      // Make a copy of the converter and set the index
      auto new_converter = converter;
      new_converter.second.index = instance.second;
      // Register the new converter with the new name
      auto& new_name = instance.first;
      m_name_to_converter_map.emplace(new_name, new_converter);
      m_name_to_col_info_map.emplace(new_name, col_info);
      new_names.push_back(new_name);
    }

    // Replace all the old names with the new ones
    auto& names = m_property_to_names_map[typeid(PropertyType)];
    names.erase(std::find(names.begin(), names.end(), current_name));
    std::copy(new_names.begin(), new_names.end(), std::back_inserter(names));
  }

  /**
   * Register into the registry the output alias so PropertyType can be serialized into the
   * output catalog
   * @tparam PropertyType
   *    The property holder
   * @param alias_name
   *    The alias that will be added as an option to "output-properties"
   * @param configurable_output
   *    By default, OutputRegistry verifies that there are column converters registered
   *    for PropertyType. If this option is set to true, the check will be skipped,
   *    and it will be assumed that the output columns will be added during the configuration
   *    of the plugin
   */
  template <typename PropertyType>
  void enableOutput(std::string alias_name, bool configurable_output = false) {
    if (m_property_to_names_map.count(typeid(PropertyType)) == 0 && !configurable_output) {
      throw Elements::Exception() << "No registered ColumnConverters for"
              << " property " << typeid(PropertyType).name();
    }
    m_output_properties.emplace(alias_name, typeid(PropertyType));
  }

  std::set<std::string> getOutputPropertyNames() const {
    std::set<std::string> result {};
    for (auto& pair : m_output_properties) {
      result.emplace(pair.first);
    }
    return result;
  }

  std::set<std::string> getColumns(const std::string& property) const;

  std::set<std::string> getColumns(const PropertyId& property) const;

  SourceToRowConverter getSourceToRowConverter(const std::vector<std::string>& enabled_optional);

  void printPropertyColumnMap(const std::vector<std::string>& properties={});

private:
  std::map<std::type_index, std::vector<std::string>>                 m_property_to_names_map{};
  std::map<std::string, std::type_index>                              m_name_to_property_map{};
  std::map<std::string, std::pair<std::type_index, ColumnFromSource>> m_name_to_converter_map{};
  std::map<std::string, ColInfo>                                      m_name_to_col_info_map{};
  std::multimap<std::string, std::type_index>                         m_output_properties{};
};

} /* namespace SourceXtractor */

#endif /* _SEFRAMEWORK_OUTPUTREGISTRY_H */

