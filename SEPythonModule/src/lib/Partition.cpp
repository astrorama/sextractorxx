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

#include "SEPythonModule/Partition.h"
#include "SEImplementation/Partition/PartitionFactory.h"
#include "SEPythonModule/PipelineReceiver.h"
#include <boost/python/extract.hpp>

namespace py = boost::python;
namespace se = SourceXtractor;

namespace SourceXPy {

Partition::Partition(ContextPtr context) : m_context(std::move(context)) {
  m_partition = m_context->m_partition_factory->getPartition();
}

std::string Partition::repr() const {
  return "Partition";
}

void Partition::setNextStage(const py::object& callback) {
  py::extract<std::shared_ptr<SourceReceiverIfce>> cpp_ifce(callback);
  if (cpp_ifce.check()) {
    m_partition->setNextStage(cpp_ifce());
  } else {
    m_partition->setNextStage(std::make_shared<PipelineSourceReceiver>(callback, m_context));
  }
}

void Partition::receiveSource(std::unique_ptr<SourceXtractor::SourceInterface> source) {
  m_partition->receiveSource(std::move(source));
}

void Partition::receiveProcessSignal(const SourceXtractor::ProcessSourcesEvent& event) {
  m_partition->receiveProcessSignal(event);
}

void Partition::call(const py::object& obj) const {
  py::extract<AttachedSource> source_wrapper(obj);
  if (source_wrapper.check()) {
    const auto&        source_ptr = source_wrapper().m_source_ptr;
    Pyston::SaveThread save_thread;
    m_partition->receiveSource(source_ptr->clone());
    return;
  }
  py::extract<se::ProcessSourcesEvent> event_wrapper(obj);
  if (event_wrapper.check()) {
    const auto&        event = event_wrapper();
    Pyston::SaveThread save_thread;
    m_partition->receiveProcessSignal(event);
    return;
  }
  PyErr_SetString(PyExc_TypeError, "Partition: Unexpected python object received");
  py::throw_error_already_set();
}

}  // namespace SourceXPy
