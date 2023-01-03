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

#include "SEPythonModule/Deblending.h"
#include "Pyston/GIL.h"
#include "SEImplementation/Deblending/DeblendingFactory.h"
#include "SEPythonModule/PipelineReceiver.h"
#include <boost/python/extract.hpp>

namespace py = boost::python;
namespace se = SourceXtractor;

namespace SourceXPy {

Deblending::Deblending(ContextPtr context) : m_context(std::move(context)) {
  m_deblending = m_context->m_deblending_factory->createDeblending();
}

std::string Deblending::repr() const {
  return "Deblending";
}

void Deblending::setNextStage(const py::object& callback) {
  m_deblending->setNextStage(std::make_shared<PipelineGroupReceiver>(callback, m_context));
}

void Deblending::receiveSource(std::unique_ptr<SourceXtractor::SourceGroupInterface> source) {
  m_deblending->receiveSource(std::move(source));
}

void Deblending::receiveProcessSignal(const SourceXtractor::ProcessSourcesEvent& event) {
  m_deblending->receiveProcessSignal(event);
}

void Deblending::call(const py::object& obj) const {
  py::extract<SourceGroup> group_wrapper(obj);
  if (group_wrapper.check()) {
    const auto&                                           group_ptr = group_wrapper().m_group;
    Pyston::SaveThread                                    save_thread;
    std::unique_ptr<SourceXtractor::SourceGroupInterface> cloned_group_ptr(
        dynamic_cast<SourceXtractor::SourceGroupInterface*>(group_ptr->clone().release()));
    m_deblending->receiveSource(std::move(cloned_group_ptr));
    return;
  }
  py::extract<se::ProcessSourcesEvent> event_wrapper(obj);
  if (event_wrapper.check()) {
    const auto&        event = event_wrapper();
    Pyston::SaveThread save_thread;
    m_deblending->receiveProcessSignal(event);
    return;
  }
  PyErr_SetString(PyExc_TypeError, "Deblending: Unexpected python object received");
  py::throw_error_already_set();
}

}  // namespace SourceXPy
