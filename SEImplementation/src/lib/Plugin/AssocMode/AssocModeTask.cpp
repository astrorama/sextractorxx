/** Copyright © 2021 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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


#include <iostream>
#include <algorithm>
#include <functional>

#include "SEUtils/KdTree.h"

#include "SEImplementation/Plugin/PixelCentroid/PixelCentroid.h"
#include "SEImplementation/Plugin/DetectionFrameInfo/DetectionFrameInfo.h"

#include "SEImplementation/Plugin/AssocMode/AssocMode.h"
#include "SEImplementation/Plugin/AssocMode/AssocModeTask.h"

namespace SourceXtractor {

//////////////////////////////////////////////////////////////////////////////////////////

template <>
struct KdTreeTraits<AssocModeConfig::CatalogEntry> {
  static double getCoord(const AssocModeConfig::CatalogEntry& t, size_t index) {
    if (index == 0) {
      return t.coord.m_x;
    } else {
      return t.coord.m_y;
    }
  }
};

namespace {

AssocModeConfig::CatalogEntry getAssocEntryUnknownImpl(const std::vector<AssocModeConfig::CatalogEntry>&) {
  throw Elements::Exception() << "UNKNOWN association mode";
}

AssocModeConfig::CatalogEntry getAssocEntryFirstImpl(const std::vector<AssocModeConfig::CatalogEntry>& entries) {
  return entries.front();
}

AssocModeConfig::CatalogEntry getAssocEntryNearestImpl(const std::vector<AssocModeConfig::CatalogEntry>& entries,
                                                       double x, double y) {
  return *std::min_element(entries.begin(), entries.end(),
      [x, y](const AssocModeConfig::CatalogEntry &a, const AssocModeConfig::CatalogEntry &b) -> bool {
          auto dx_a = a.coord.m_x - x;
          auto dy_a = a.coord.m_y - y;
          auto dx_b = b.coord.m_x - x;
          auto dy_b = b.coord.m_y - y;
          return dx_a*dx_a + dy_a*dy_a < dx_b*dx_b + dy_b*dy_b;
      });
}

AssocModeConfig::CatalogEntry getAssocEntryMinImpl(const std::vector<AssocModeConfig::CatalogEntry>& entries) {
  return *std::min_element(entries.begin(), entries.end(),
      [](const AssocModeConfig::CatalogEntry &a, const AssocModeConfig::CatalogEntry &b) -> bool {
          return a.weight < b.weight;
      });
}

AssocModeConfig::CatalogEntry getAssocEntryMaxImpl(const std::vector<AssocModeConfig::CatalogEntry>& entries) {
  return *std::max_element(entries.begin(), entries.end(),
      [](const AssocModeConfig::CatalogEntry &a, const AssocModeConfig::CatalogEntry &b) -> bool {
          return a.weight < b.weight;
      });
}

AssocModeConfig::CatalogEntry getAssocEntryMeanImpl(const std::vector<AssocModeConfig::CatalogEntry>& entries) {
  AssocModeConfig::CatalogEntry result{{}, {}, 0., {}, 0., 0., 0};
  result.assoc_columns.resize(entries.front().assoc_columns.size());
  double total_weight = 0.0;
  for (auto& entry : entries) {
    total_weight += entry.weight;
    for (size_t i=0; i < result.assoc_columns.size(); i++) {
      result.assoc_columns[i] += entry.assoc_columns[i] * entry.weight;
    }
  }

  for (size_t i=0; i < result.assoc_columns.size(); i++) {
    result.assoc_columns[i] /= total_weight;
  }

  return result;
}

AssocModeConfig::CatalogEntry getAssocEntrySumImpl(const std::vector<AssocModeConfig::CatalogEntry>& entries) {
  AssocModeConfig::CatalogEntry result{{}, {}, 0., {}, 0., 0., 0};
  result.assoc_columns.resize(entries.front().assoc_columns.size());
  for (auto& entry : entries) {
    for (size_t i=0; i < result.assoc_columns.size(); i++) {
      result.assoc_columns[i] += entry.assoc_columns[i];
    }
  }
  return result;
}

AssocModeConfig::CatalogEntry getAssocEntryMagMeanImpl(const std::vector<AssocModeConfig::CatalogEntry>& entries) {
  AssocModeConfig::CatalogEntry result{{}, {}, 0., {}, 0., 0., 0};
  result.assoc_columns.resize(entries.front().assoc_columns.size());
  double total_weight = 0.0;
  for (auto& entry : entries) {
    double linear_weight = pow(10.0, -0.4 * entry.weight);
    total_weight += linear_weight;
    for (size_t i=0; i < result.assoc_columns.size(); i++) {
      result.assoc_columns[i] += entry.assoc_columns[i] * linear_weight;
    }
  }

  for (size_t i=0; i < result.assoc_columns.size(); i++) {
    result.assoc_columns[i] /= total_weight;
  }

  return result;
}

AssocModeConfig::CatalogEntry getAssocEntryMagSumImpl(const std::vector<AssocModeConfig::CatalogEntry>& entries) {
  AssocModeConfig::CatalogEntry result{{}, {}, 0., {}, 0., 0., 0};
  result.assoc_columns.resize(entries.front().assoc_columns.size());
  for (auto& entry : entries) {
    for (size_t i=0; i < result.assoc_columns.size(); i++) {
      result.assoc_columns[i] += pow(10.0, -0.4 * entry.assoc_columns[i]);
    }
  }

  for (size_t i=0; i < result.assoc_columns.size(); i++) {
    result.assoc_columns[i] = -2.5 * log10(result.assoc_columns[i]);
  }

  return result;
}

}

AssocModeTask::AssocModeTask(const std::vector<std::vector<AssocModeConfig::CatalogEntry>>& catalogs,
                             AssocModeConfig::AssocMode assoc_mode, double radius) :
                             m_assoc_mode(assoc_mode), m_radius(radius) {
  for (auto& catalog : catalogs) {
    m_catalogs.emplace_back(catalog);
  }
}

void AssocModeTask::computeProperties(SourceInterface &source) const {
  using namespace std::placeholders;  // for _1, _2, _3...
  using Tree = KdTree<AssocModeConfig::CatalogEntry>;

  // get the object center
  const auto& x = source.getProperty<PixelCentroid>().getCentroidX();
  const auto& y = source.getProperty<PixelCentroid>().getCentroidY();

  auto hdu_index = source.getProperty<DetectionFrameInfo>().getHduIndex();
  const auto& catalog = m_catalogs.at(hdu_index);

  auto nearby_catalog_entries = catalog.findPointsWithinRadius(Tree::Coord { x, y }, m_radius);

  if (nearby_catalog_entries.size() == 0) {
    // No match
    source.setProperty<AssocMode>(false, std::vector<double>());
  } else {
    const std::map<AssocModeConfig::AssocMode, AssocModeTask::GetAssocResult> assoc_mode_implementation_table {
      std::make_pair(AssocModeConfig::AssocMode::UNKNOWN, getAssocEntryUnknownImpl),
      std::make_pair(AssocModeConfig::AssocMode::FIRST, getAssocEntryFirstImpl),
      std::make_pair(AssocModeConfig::AssocMode::NEAREST, std::bind(getAssocEntryNearestImpl, _1, x, y)),
      std::make_pair(AssocModeConfig::AssocMode::MIN, getAssocEntryMinImpl),
      std::make_pair(AssocModeConfig::AssocMode::MAX, getAssocEntryMaxImpl),
      std::make_pair(AssocModeConfig::AssocMode::MEAN, getAssocEntryMeanImpl),
      std::make_pair(AssocModeConfig::AssocMode::SUM, getAssocEntrySumImpl),
      std::make_pair(AssocModeConfig::AssocMode::MAG_SUM, getAssocEntryMagSumImpl),
      std::make_pair(AssocModeConfig::AssocMode::MAG_MEAN, getAssocEntryMagMeanImpl)
    };

    auto assoc_data = assoc_mode_implementation_table.at(m_assoc_mode)(nearby_catalog_entries);
    source.setProperty<AssocMode>(true, assoc_data.assoc_columns);
  }
}

}
