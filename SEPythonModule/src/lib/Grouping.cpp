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

#include "SEPythonModule/Grouping.h"
#include "SEImplementation/Grouping/GroupingFactory.h"
#include "SEPythonModule/PipelineReceiver.h"
#include <boost/python/extract.hpp>

namespace py = boost::python;
namespace se = SourceXtractor;

namespace SourceXPy {

Grouping::Grouping(ContextPtr context) : m_context(std::move(context)) {
  m_grouping = m_context->m_grouping_factory->createGrouping();
}

std::string Grouping::repr() const {
  return "Grouping";
}

void Grouping::setNextStage(const py::object& callback) {
  py::extract<std::shared_ptr<GroupReceiverIfce>> cpp_ifce(callback);
  if (cpp_ifce.check()) {
    m_grouping->setNextStage(cpp_ifce());
  } else {
    m_grouping->setNextStage(std::make_shared<PipelineGroupReceiver>(callback, m_context));
  }
}

void Grouping::receiveSource(std::unique_ptr<SourceXtractor::SourceInterface> source) {
  m_grouping->receiveSource(std::move(source));
}

void Grouping::receiveProcessSignal(const SourceXtractor::ProcessSourcesEvent& event) {
  m_grouping->receiveProcessSignal(event);
}

void Grouping::call(const py::object& obj) const {
  py::extract<AttachedSource> source_wrapper(obj);
  if (source_wrapper.check()) {
    const auto&        source_ptr = source_wrapper().m_source_ptr;
    Pyston::SaveThread save_thread;
    m_grouping->receiveSource(source_ptr->clone());
    return;
  }
  py::extract<se::ProcessSourcesEvent> event_wrapper(obj);
  if (event_wrapper.check()) {
    const auto&        event = event_wrapper();
    Pyston::SaveThread save_thread;
    m_grouping->receiveProcessSignal(event);
    return;
  }
  PyErr_SetString(PyExc_TypeError, "Grouping: Unexpected python object received");
  py::throw_error_already_set();
}

}  // namespace SourceXPy
