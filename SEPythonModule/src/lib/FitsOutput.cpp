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

#include "SEPythonModule/FitsOutput.h"
#include "SEImplementation/Output/OutputFactory.h"
#include "SEPythonModule/PipelineReceiver.h"
#include "SEPythonModule/SourceInterface.h"
#include <boost/python/extract.hpp>

namespace py = boost::python;
namespace se = SourceXtractor;

namespace SourceXPy {

FitsOutput::FitsOutput(SourceXPy::ContextPtr context) : m_context(std::move(context)) {
  m_output = m_context->m_output_factory->createOutput();
}

FitsOutput::~FitsOutput() = default;

std::string FitsOutput::repr() const {
  return "FitsOutput";
}

void FitsOutput::receiveSource(std::unique_ptr<SourceXtractor::SourceGroupInterface> group) {
  m_output->receiveSource(std::move(group));
}

void FitsOutput::receiveProcessSignal(const SourceXtractor::ProcessSourcesEvent& event) {
  m_output->receiveProcessSignal(event);
  auto all_done = std::dynamic_pointer_cast<AllFramesDone>(event.m_selection_criteria);
  if (all_done) {
    m_semaphore.release();
  }
}

void FitsOutput::call(const boost::python::object& obj) {
  py::extract<SourceGroup> group(obj);
  if (group.check()) {
    const auto&                                           group_ptr = group().m_group;
    Pyston::SaveThread                                    save_thread;
    std::unique_ptr<SourceXtractor::SourceGroupInterface> cloned_group_ptr(
        dynamic_cast<SourceXtractor::SourceGroupInterface*>(group_ptr->clone().release()));
    m_output->receiveSource(std::move(cloned_group_ptr));
    return;
  }
  py::extract<se::ProcessSourcesEvent> event_wrapper(obj);
  if (event_wrapper.check()) {
    const auto&        event = event_wrapper();
    Pyston::SaveThread save_thread;
    m_output->receiveProcessSignal(event);
    return;
  }
  PyErr_SetString(PyExc_TypeError, "FitsOutput: Unexpected python object received");
  py::throw_error_already_set();
}

void FitsOutput::get(std::chrono::microseconds timeout) {
  static constexpr std::chrono::seconds try_wait(1);
  Pyston::SaveThread                    save_thread;
  while (!m_semaphore.try_acquire_for(try_wait)) {
    m_context->m_thread_pool->checkForException(true);
    timeout -= try_wait;
    if (timeout <= std::chrono::microseconds::zero()) {
      Pyston::GILLocker gil;
#if PY_VERSION_HEX >= 0x03030000
      PyErr_SetString(PyExc_TimeoutError, "sourcextractor timed-out");
#else
      PyErr_SetString(PyExc_RuntimeError, "sourcextractor timed-out");
#endif
      py::throw_error_already_set();
    }
  }
  m_context->m_thread_pool->checkForException(true);
}

}  // namespace SourceXPy
