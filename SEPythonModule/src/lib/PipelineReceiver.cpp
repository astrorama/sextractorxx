/**
 * Copyright © 2022 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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

#include "SEPythonModule/PipelineReceiver.h"

namespace se = SourceXtractor;

namespace SourceXPy {

std::string ProcessSourcesEventRepr(const se::ProcessSourcesEvent& event) {
  auto& ptr = event.m_selection_criteria;
  if (std::dynamic_pointer_cast<se::SelectAllCriteria>(ptr)) {
    return "SelectAllCriteria";
  } else if (std::dynamic_pointer_cast<AllFramesDone>(ptr)) {
    return "AllFramesDone";
  }
  return "LineSelectionCriteria";
}

bool ProcessSourcesEventMustProcess(const SourceXtractor::ProcessSourcesEvent& event, const AttachedSource& source) {
  return event.m_selection_criteria->mustBeProcessed(*source.m_source_ptr);
}

bool AllFramesDone::mustBeProcessed(const se::SourceInterface&) const {
  return true;
}

se::ProcessSourcesEvent AllFramesDone::create() {
  return se::ProcessSourcesEvent{std::make_shared<AllFramesDone>()};
}

}  // namespace SourceXPy
