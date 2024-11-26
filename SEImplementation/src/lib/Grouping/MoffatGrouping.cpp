/** Copyright © 2019-2024 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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

#include "SEUtils/QuadTree.h"

#include <set>

#include "SEImplementation/Grouping/MoffatGrouping.h"
#include "SEImplementation/Grouping/MoffatCriteria.h"

#include "SEImplementation/Plugin/MoffatModelFitting/MoffatModelFitting.h"
#include "SEImplementation/Plugin/MoffatModelFitting/MoffatModelEvaluator.h"
#include "SEImplementation/Plugin/PixelCentroid/PixelCentroid.h"
#include "SEImplementation/Plugin/PeakValue/PeakValue.h"

namespace SourceXtractor {

template <>
struct QuadTreeTraits<std::shared_ptr<MoffatGrouping::SourceInfo>> {
  static double getCoord(std::shared_ptr<MoffatGrouping::SourceInfo> t, size_t index) {
    if (index == 0) {
      return t->m_x;
    } else {
      return t->m_y;
    }
  }
};


MoffatGrouping::MoffatGrouping(
    std::shared_ptr<GroupingCriteria> grouping_criteria, std::shared_ptr<SourceGroupFactory> group_factory, unsigned int hard_limit, float max_range)
    : m_grouping_criteria(grouping_criteria), m_group_factory(group_factory), m_hard_limit(hard_limit), m_max_range(max_range), m_group_counter(0)

{
}

std::set<PropertyId> MoffatGrouping::requiredProperties() const {
  return {
    PropertyId::create<PixelCentroid>(),
    PropertyId::create<PeakValue>(),
    PropertyId::create<MoffatModelEvaluator>(),
  };
}

/// Handles a new Source
void MoffatGrouping::receiveSource(std::unique_ptr<SourceInterface> source) {

  // Encapsulates the source unique_ptr
  auto& centroid = source->getProperty<PixelCentroid>();

  // Creates a SourceInfo to contain the unique_ptr to the source, coordinates and group_id
  auto source_info = std::make_shared<SourceInfo>();
  source_info->m_source = std::move(source);
  source_info->m_x = centroid.getCentroidX();
  source_info->m_y = centroid.getCentroidY();
  source_info->m_group_id = m_group_counter++;

  // Make a group containing only the source
  auto group = std::make_shared<Group>();
  group->push_back(source_info);
  m_groups[source_info->m_group_id] = group;

  // Find sources within range
  auto sources = m_tree.getPointsWithinRange({source_info->m_x, source_info->m_y}, m_max_range);

  std::set<size_t> matching_groups;
  for (auto& s : sources) {
    // check only sources that belong to a group we don't match with
    if (matching_groups.find(s->m_group_id) == matching_groups.end() &&
        m_grouping_criteria->shouldGroup(*source_info->m_source, *s->m_source)) {
      matching_groups.insert(s->m_group_id);
    }
  }

  // merge groups and keep the new group
  for (auto group_id : matching_groups) {
    if (m_hard_limit > 0 && m_groups.at(group_id)->size() + group->size() > m_hard_limit) {
      // if we have a hard limit and the group is too large to merge, just process it
      processGroup(group_id);
    } else {
      // merge group
      group->insert(group->end(), m_groups.at(group_id)->begin(), m_groups.at(group_id)->end());
      m_groups.erase(group_id);
    }
  }

  for (auto& s : *group) {
    s->m_group_id = source_info->m_group_id;
  }

  // Add source to the Quad Tree
  m_tree.add(source_info);
}

/// Handles a ProcessSourcesEvent to trigger the processing of some of the Sources stored in SourceGrouping
void MoffatGrouping::receiveProcessSignal(const ProcessSourcesEvent& event) {
  std::vector<size_t> groups_to_process;

  // We iterate through all the SourceGroups we have
  for (auto const& it : m_groups) {
    // We look at its Sources and if we find at least one that needs to be processed
    for (auto& source : *it.second) {
      if (event.m_selection_criteria->mustBeProcessed(*source->m_source)) {
        groups_to_process.push_back(it.first);
        break;
      }
    }
  }

  // For each SourceGroup that we put in groups_to_process,
  for (auto group_id : groups_to_process) {
    processGroup(group_id);
  }
}

void MoffatGrouping::processGroup(unsigned int group_id) {
  // we remove it from our list of stored SourceGroups and notify our observers
  auto new_group = m_group_factory->createSourceGroup();

  for (auto& source_info : *m_groups.at(group_id)) {
    new_group->addSource(std::move(source_info->m_source));
    m_tree.remove(source_info);
  }

  sendSource(std::move(new_group));
  m_groups.erase(group_id);
}

} // SourceXtractor namespace

