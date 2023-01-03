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
 * File:   OutputRegistry.cpp
 * Author: nikoapos
 *
 * Created on July 28, 2016, 7:12 PM
 */

#include <algorithm>

#include "ElementsKernel/Exception.h"

#include "SEFramework/Output/OutputRegistry.h"

#include <iostream>

using namespace Euclid::Table;

namespace SourceXtractor {

auto OutputRegistry::getSourceToRowConverter(const std::vector<std::string>& enabled_properties)
    -> SourceToRowConverter {
  std::vector<std::type_index> out_prop_list{};
  for (auto& prop : enabled_properties) {
    if (m_output_properties.count(prop) == 0) {
      throw Elements::Exception() << "Unknown output property " << prop;
    }
    auto matching_properties = m_output_properties.equal_range(prop);
    for (auto i = matching_properties.first; i != matching_properties.second; ++i) {
      if (std::find(out_prop_list.begin(), out_prop_list.end(), i->second) == out_prop_list.end()) {
        out_prop_list.emplace_back(i->second);
      }
    }
  }
  return [this, out_prop_list](const SourceInterface& source) {
    std::vector<ColumnInfo::info_type> info_list{};
    std::vector<Row::cell_type>        cell_values{};
    for (const auto& property : out_prop_list) {
      if (m_property_to_names_map.count(property) == 0) {
        throw Elements::Exception() << "Missing column generator for " << property.name();
      }
      for (const auto& name : m_property_to_names_map.at(property)) {
        auto& col_info = m_name_to_col_info_map.at(name);
        info_list.emplace_back(name, m_name_to_converter_map.at(name).first, col_info.unit, col_info.description);
        cell_values.emplace_back(m_name_to_converter_map.at(name).second(source));
      }
    }
    if (info_list.empty()) {
      throw Elements::Exception() << "The given configuration would not generate any output";
    }
    return Row{std::move(cell_values), std::make_shared<ColumnInfo>(move(info_list))};
  };
}

void OutputRegistry::printPropertyColumnMap(const std::vector<std::string>& properties) {
  std::set<std::string> properties_set{properties.begin(), properties.end()};
  for (auto& prop : m_output_properties) {
    if (!properties.empty() && properties_set.find(prop.first) == properties_set.end()) {
      continue;
    }
    std::cout << prop.first << ":\n";
    for (auto& col : m_property_to_names_map.at(prop.second)) {
      std::cout << "  - " << col;
      auto& info = m_name_to_col_info_map.at(col);
      if (info.description != "") {
        std::cout << " : " << info.description;
      }
      if (info.unit != "") {
        std::cout << " " << info.unit << "";  // place here braces "()" around the units, if desired
      }
      std::cout << '\n';
    }
  }
}

std::set<std::string> OutputRegistry::getColumns(const std::string& property) const {
  std::set<std::string> columns;
  auto                  iter_range = m_output_properties.equal_range(property);

  for (auto i = iter_range.first; i != iter_range.second; ++i) {
    const auto& attrs = m_property_to_names_map.at(i->second);
    std::copy(attrs.begin(), attrs.end(), std::inserter(columns, columns.begin()));
  }

  return columns;
}

std::set<std::string> OutputRegistry::getColumns(const PropertyId& property) const {
  auto iter = m_property_to_names_map.find(property.getTypeId());
  if (iter != m_property_to_names_map.end()) {
    return {iter->second.begin(), iter->second.end()};
  }
  return {};
}

}  // namespace SourceXtractor
