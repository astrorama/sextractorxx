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

#include "SEPythonModule/Measurement.h"
#include "SEImplementation/Measurement/MeasurementFactory.h"
#include "SEPythonModule/PipelineReceiver.h"
#include <boost/python/extract.hpp>

namespace py = boost::python;
namespace se = SourceXtractor;

namespace SourceXPy {

Measurement::Measurement(ContextPtr context) : m_context(std::move(context)) {
  m_measurement = m_context->m_measurement_factory->getMeasurement();
}

Measurement::~Measurement() {
  try {
    m_measurement->cancel();
    m_measurement->stopThreads();
  } catch (...) {
    // Must not throw from a destructor
  }
}

std::string Measurement::repr() const {
  return "Measurement";
}

void Measurement::setNextStage(const boost::python::object& callback) {
  py::extract<std::shared_ptr<GroupReceiverIfce>> cpp_ifce(callback);
  if (cpp_ifce.check()) {
    m_measurement->setNextStage(cpp_ifce());
  } else {
    m_measurement->setNextStage(std::make_shared<PipelineGroupReceiver>(callback, m_context));
  }
  m_measurement->startThreads();
}

void Measurement::receiveSource(std::unique_ptr<SourceXtractor::SourceGroupInterface> group) {
  m_measurement->receiveSource(std::move(group));
}

void Measurement::receiveProcessSignal(const SourceXtractor::ProcessSourcesEvent& event) {
  m_measurement->receiveProcessSignal(event);
}

void Measurement::call(const py::object& obj) {
  py::extract<SourceGroup> group(obj);
  if (group.check()) {
    const auto&                                           group_ptr = group().m_group;
    Pyston::SaveThread                                    save_thread;
    std::unique_ptr<SourceXtractor::SourceGroupInterface> cloned_group_ptr(
        dynamic_cast<SourceXtractor::SourceGroupInterface*>(group_ptr->clone().release()));
    m_measurement->receiveSource(std::move(cloned_group_ptr));
    return;
  }
  py::extract<se::ProcessSourcesEvent> event_wrapper(obj);
  if (event_wrapper.check()) {
    const auto&        event = event_wrapper();
    Pyston::SaveThread save_thread;
    m_measurement->receiveProcessSignal(event);
    return;
  }
  PyErr_SetString(PyExc_TypeError, "Measurement: Unexpected python object received");
  py::throw_error_already_set();
}

}  // namespace SourceXPy
