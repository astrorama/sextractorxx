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

#include "SEPythonModule/Context.h"
#include "SEPythonModule/Deblending.h"
#include "SEPythonModule/FitsOutput.h"
#include "SEPythonModule/Grouping.h"
#include "SEPythonModule/Measurement.h"
#include "SEPythonModule/NumpyOutput.h"
#include "SEPythonModule/Partition.h"
#include "SEPythonModule/PipelineReceiver.h"
#include "SEPythonModule/Segmentation.h"
#include "SEPythonModule/SourceInterface.h"
#include <Pyston/Module.h>
#include <boost/python/args.hpp>
#include <boost/python/class.hpp>
#include <boost/python/module.hpp>
#include <boost/python/numpy.hpp>
#include <boost/python/register_ptr_to_python.hpp>
#include <datetime.h>

using namespace SourceXPy;
namespace py = boost::python;
namespace np = boost::python::numpy;
namespace se = SourceXtractor;

namespace {

/**
 * Pickle detached sources
 */
struct PickleDetachedSource : public py::pickle_suite {
  static py::tuple getinitargs(const DetachedSource&) {
    return py::make_tuple();
  }

  static py::tuple getstate(const DetachedSource& source) {
    return py::make_tuple(source.m_attributes);
  }

  static void setstate(DetachedSource& source, py::tuple state) {
    source.m_attributes = py::extract<py::dict>(state[0]);
  }
};

/**
 * Convert std::chrono::duration to datetime.timedelta
 */
struct timedelta_from_std_duration {
  timedelta_from_std_duration() {
    py::to_python_converter<const std::chrono::microseconds, timedelta_from_std_duration>();
  }

  static PyObject* convert(std::chrono::microseconds us) {
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(us);
    us -= seconds;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(seconds);
    seconds -= hours;
    return PyDelta_FromDSU(hours.count() / 24, seconds.count(), us.count());
  }
};

/**
 * Convert datetime.timedelta to std::chrono::duration
 */
struct std_duration_from_timedelta {
  std_duration_from_timedelta() {
    py::converter::registry::push_back(&convertible, &construct, py::type_id<std::chrono::microseconds>());
  }

  static void* convertible(PyObject* obj_ptr) {
    if (PyDelta_Check(obj_ptr)) {
      return obj_ptr;
    }
    return nullptr;
  }

  static void construct(PyObject* obj_ptr, py::converter::rvalue_from_python_stage1_data* data) {
    auto timedelta    = reinterpret_cast<PyDateTime_Delta*>(obj_ptr);
    auto hours        = std::chrono::hours(timedelta->days * 24l);
    auto seconds      = std::chrono::seconds(timedelta->seconds);
    auto microseconds = std::chrono::microseconds(timedelta->microseconds);
    auto duration     = hours + seconds + microseconds;

    auto storage =
        (reinterpret_cast<py::converter::rvalue_from_python_storage<std::chrono::microseconds>*>(data))->storage.bytes;
    new (storage) std::chrono::microseconds(duration);
    data->convertible = storage;
  }
};

void translate_pyston_exc(const Pyston::Exception& exc) {
  exc.restore();
}

}  // namespace

BOOST_PYTHON_MODULE(_SEPythonModule) {
  np::initialize();
  py::object None;

  PyDateTime_IMPORT;
  std_duration_from_timedelta();
  timedelta_from_std_duration();

  py::class_<Context, boost::noncopyable>(
      "Context", py::init<py::dict, py::object>((py::arg("config"), py::arg("measurement_config") = py::object())))
      .def("get_properties", &Context::get_properties)
      .def("check_exception", &Context::check_exception)
      .def("__enter__", &Context::enter)
      .def("__exit__", &Context::exit);
  py::register_ptr_to_python<std::shared_ptr<Context>>();

  py::class_<DetachedSource>("DetachedSource")
      .def("__repr__", &DetachedSource::repr)
      .def("__getattr__", &DetachedSource::attribute)
      .def("__dir__", &DetachedSource::attributes)
      .def_pickle(PickleDetachedSource());

  py::class_<AttachedSource, boost::noncopyable>("Source", py::no_init)
      .def("__getattr__", &AttachedSource::attribute)
      .def("detach", &AttachedSource::detach)
      .def("create", &OwnedSource::create, py::args("context", "detection_frame", "detection_id", "pixel_coordinates"))
      .staticmethod("create");
  py::register_ptr_to_python<std::shared_ptr<AttachedSource>>();

  py::class_<OwnedSource, py::bases<AttachedSource>, boost::noncopyable>("OwnedSource", py::no_init)
      .def("__repr__", &OwnedSource::repr);
  py::register_ptr_to_python<std::shared_ptr<OwnedSource>>();

  py::class_<EntangledSource, py::bases<AttachedSource>, boost::noncopyable>("EntangledSource", py::no_init)
      .def("__repr__", &EntangledSource::repr);

  py::register_ptr_to_python<std::shared_ptr<EntangledSource>>();

  py::class_<SourceGroup::Iterator>("GroupIterator", py::no_init).def("__next__", &SourceGroup::Iterator::next);

  py::class_<SourceGroup, boost::noncopyable>("Group", py::no_init)
      .def("__repr__", &SourceGroup::repr)
      .def("__getattr__", &SourceGroup::attribute)
      .def("__len__", &SourceGroup::size)
      .def("__iter__", &SourceGroup::iter)
      .def("create", &SourceGroup::create)
      .staticmethod("create");
  py::register_ptr_to_python<std::shared_ptr<SourceGroup>>();

  py::class_<se::ProcessSourcesEvent>("ProcessSourcesEvent", py::no_init)
      .def("__repr__", &ProcessSourcesEventRepr)
      .def("must_process", &ProcessSourcesEventMustProcess);

  py::class_<SourceReceiverIfce, boost::noncopyable>("SourceReceiver", py::no_init);
  py::class_<GroupReceiverIfce, boost::noncopyable>("GroupReceiverIfce", py::no_init);

  py::class_<Segmentation>("Segmentation", py::init<std::shared_ptr<Context>>((py::arg("context") = None)))
      .def("__repr__", &Segmentation::repr)
      .def("set_next_stage", &Segmentation::setNextStage)
      .def("__call__", &Segmentation::call);

  py::class_<Partition, py::bases<SourceReceiverIfce>>("Partition",
                                                       py::init<std::shared_ptr<Context>>((py::arg("context") = None)))
      .def("__repr__", &Partition::repr)
      .def("set_next_stage", &Partition::setNextStage)
      .def("__call__", &Partition::call);

  py::class_<Grouping, py::bases<SourceReceiverIfce>>("Grouping",
                                                      py::init<std::shared_ptr<Context>>((py::arg("context") = None)))
      .def("__repr__", &Grouping::repr)
      .def("set_next_stage", &Grouping::setNextStage)
      .def("__call__", &Grouping::call);

  py::class_<Deblending, py::bases<GroupReceiverIfce>>("Deblending",
                                                       py::init<std::shared_ptr<Context>>((py::arg("context") = None)))
      .def("__repr__", &Deblending::repr)
      .def("set_next_stage", &Deblending::setNextStage)
      .def("__call__", &Deblending::call);

  py::class_<Measurement, py::bases<GroupReceiverIfce>, boost::noncopyable>(
      "Measurement", py::init<std::shared_ptr<Context>>((py::arg("context") = None)))
      .def("__repr__", &Measurement::repr)
      .def("set_next_stage", &Measurement::setNextStage)
      .def("__call__", &Measurement::call);

  py::class_<NumpyOutput, py::bases<GroupReceiverIfce>, boost::noncopyable>(
      "NumpyOutput", py::init<std::shared_ptr<Context>>((py::arg("Context") = None)))
      .def("__repr__", &NumpyOutput::repr)
      .def("__call__", &NumpyOutput::call)
      .def("get", &NumpyOutput::getTable, (py::arg("timeout") = std::chrono::microseconds::max()));

  py::class_<FitsOutput, py::bases<GroupReceiverIfce>, boost::noncopyable>(
      "FitsOutput", py::init<std::shared_ptr<Context>>((py::arg("Context") = None)))
      .def("__repr__", &FitsOutput::repr)
      .def("__call__", &FitsOutput::call)
      .def("get", &FitsOutput::get, (py::arg("timeout") = std::chrono::microseconds::max()));

  // For custom segmentation
  py::def("AllFramesDone", &AllFramesDone::create);

  // Pyston::Exception wrap Python errors, so unwrap them at the Python boundary
  py::register_exception_translator<Pyston::Exception>(&translate_pyston_exc);

  // Import pyston into the interpreter so it is importable without tweaking PYTHONPATH
#if PY_MAJOR_VERSION >= 3
  PyObject* pyston = PyInit_pyston();
#else
  initpyston();
  PyObject* pyston = PyImport_ImportModule("_SEPythonConfig");
#endif
  PyObject* modules = PyImport_GetModuleDict();
  PyDict_SetItemString(modules, "pyston", pyston);
  Py_DECREF(pyston);
}
