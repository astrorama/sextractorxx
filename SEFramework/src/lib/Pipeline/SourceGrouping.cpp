/** Copyright © 2019 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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
 * @file src/lib/Pipeline/SourceGrouping.cpp
 * @date 05/24/16
 * @author mschefer
 */

#include "SEFramework/Pipeline/SourceGrouping.h"


namespace SourceXtractor {

SourceGrouping::SourceGrouping(std::shared_ptr<GroupingCriteria> grouping_criteria,
                               std::shared_ptr<SourceGroupFactory> group_factory,
                               unsigned int hard_limit)
        : m_grouping_criteria(grouping_criteria), m_group_factory(group_factory), m_hard_limit(hard_limit) {
}

void SourceGrouping::receiveSource(std::unique_ptr<SourceInterface> source) {
  // Pointer which points to the group of the source
  SourceGroupInterface* matched_group = nullptr;

  auto source_ptr = source.get();
  std::vector<std::list<std::unique_ptr<SourceGroupInterface>>::iterator> groups_to_remove {};

  for (auto group_it = m_source_groups.begin(); group_it != m_source_groups.end(); ++group_it) {

    if (m_hard_limit > 0) {
      unsigned int current_group_size = (matched_group != nullptr) ? matched_group->size() : 1;
      if (current_group_size >= m_hard_limit) {
        break; // no need to try to find matching groups anymore, we have reached the limit
      }

      if (current_group_size + (*group_it)->size() > m_hard_limit) {
        continue; // we can't merge groups without hitting the limit, so skip it
      }
    }

    // Search if the source meets the grouping criteria with any of the sources in the group
    bool in_group = false;
    for (auto& s : **group_it) {
      if (m_grouping_criteria->shouldGroup(*source_ptr, s)) {
        in_group = true;
        break; // No need to check the rest of the group sources
      }
    }

    if (in_group) {
      if (matched_group == nullptr) {
        matched_group = group_it->get();
        matched_group->addSource(std::move(source));
      } else {
        matched_group->merge(std::move(**group_it));
        groups_to_remove.emplace_back(group_it);
      }
    }
  }

  // If there was no group the source should be grouped in, we create a new one
  if (matched_group == nullptr) {
    auto new_group = m_group_factory->createSourceGroup();
    new_group->addSource(std::move(source));
    m_source_groups.emplace_back(std::move(new_group));
  }

  for (auto& group_it : groups_to_remove) {
    m_source_groups.erase(group_it);
  }
}

void SourceGrouping::receiveProcessSignal(const ProcessSourcesEvent& process_event) {
  std::vector<std::list<std::unique_ptr<SourceGroupInterface>>::iterator> groups_to_process;

  // We iterate through all the SourceGroups we have
  for (auto group_it = m_source_groups.begin(); group_it != m_source_groups.end(); ++group_it) {
    // We look at its Sources and if we find at least one that needs to be processed we put it in groups_to_process
    for (auto& source : **group_it) {
      if (process_event.m_selection_criteria->mustBeProcessed(source)) {
        groups_to_process.push_back(group_it);
        break;
      }
    }
  }

  // For each SourceGroup that we put in groups_to_process,
  for (auto& group : groups_to_process) {
    // we remove it from our list of stored SourceGroups and notify our observers
    sendSource(std::move(*group));
    m_source_groups.erase(group);
  }

  sendProcessSignal(process_event);
}

std::set<PropertyId> SourceGrouping::requiredProperties() const {
  return m_grouping_criteria->requiredProperties();
}

} // SEFramework namespace



