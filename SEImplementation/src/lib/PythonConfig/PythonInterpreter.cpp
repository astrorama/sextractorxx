/**
 * Copyright © 2019-2022 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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
/*
 * @file PythonInterpreter.cpp
 * @author Nikolaos Apostolakos <nikoapos@gmail.com>
 */

#include <Pyston/Exceptions.h>
#include <Pyston/GIL.h>
#include <Pyston/Module.h>
#include <Python.h>
#include <SEImplementation/PythonConfig/PythonInterpreter.h>
#include <SEImplementation/PythonConfig/PythonModule.h>
#include <boost/python/dict.hpp>
#include <boost/python/exec.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/import.hpp>
#include <boost/python/object.hpp>
#include <fstream>
#include <signal.h>
#include <system_error>
#include <utility>

namespace py = boost::python;

static Elements::Logging logger        = Elements::Logging::getLogger("Python::Interpreter");
static Elements::Logging stdout_logger = Elements::Logging::getLogger("Python::stdout");
static Elements::Logging stderr_logger = Elements::Logging::getLogger("Python::stderr");

namespace SourceXtractor {

PythonInterpreter& PythonInterpreter::getSingleton() {
  static PythonInterpreter singleton{};
  return singleton;
}

PythonInterpreter::PythonInterpreter() : m_out_wrapper(stdout_logger), m_err_wrapper(stderr_logger) {
  // We may be called *from* Python!
  if (!Py_IsInitialized()) {
    struct sigaction sigint_handler;

    // Python sets its own signal handler for SIGINT (Ctrl+C), so it can throw a KeyboardInterrupt
    // Here we are not interested on this behaviour, so we get whatever handler we've got (normally
    // the default one) and restore it after initializing the interpreter
    sigaction(SIGINT, nullptr, &sigint_handler);

    // Make pyston importable
    PyImport_AppendInittab("pyston", PYSTON_MODULE_INIT);

    // Initialize Python
    Py_Initialize();
#if PY_VERSION_HEX < 3090000
    PyEval_InitThreads();
#endif
    PyEval_SaveThread();

    // Restore SIGINT handler
    sigaction(SIGINT, &sigint_handler, nullptr);
  }

  // Import ourselves so the conversions are registered
  Pyston::GILLocker locker;
  py::import("_SEPythonConfig");
}

PythonInterpreter::~PythonInterpreter() {
  logger.info() << "Python GIL acquired " << Pyston::GILLocker::getLockCount() << " times";
}

void PythonInterpreter::runFile(const std::string& filename, const std::vector<std::string>& argv) {
  Pyston::GILLocker locker;

  try {
    // Setup argv
    // Python expects to have the ownership!
#if PY_MAJOR_VERSION == 2
    using py_argv_char_t = char;
#define py_argv_assign(d, s, l) d = strndup(s, l)
#else
    using py_argv_char_t = wchar_t;
#define py_argv_assign(d, s, l) d = Py_DecodeLocale(s, &(l))
#endif

    py_argv_char_t** py_argv = static_cast<py_argv_char_t**>(PyMem_MALLOC((argv.size() + 1) * sizeof(py_argv_char_t*)));
    size_t           wlen    = filename.size();
    py_argv_assign(py_argv[0], filename.c_str(), wlen);
    for (size_t i = 0; i < argv.size(); ++i) {
      wlen = argv[i].size();
      py_argv_assign(py_argv[i + 1], argv[i].c_str(), wlen);
    }
    PySys_SetArgv(argv.size() + 1, py_argv);

    // Run the file
    py::object main_module = py::import("__main__");
    py::setattr(main_module, "__file__", py::object(filename));
    py::object main_namespace = main_module.attr("__dict__");

    // boost 1.75 up to 1.77 has a bug that trashes the heap
    // See https://github.com/boostorg/python/issues/371
    // So we read the file ourselves as a workaround
    std::ifstream fs(filename);
    if (fs.fail()) {
      throw std::system_error(errno, std::system_category(), filename);
    }
    std::string pycode((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
    py::exec(pycode.c_str(), main_namespace);
  } catch (const py::error_already_set& e) {
    throw Pyston::Exception().log(log4cpp::Priority::ERROR, logger);
  } catch (const std::system_error& e) {
    throw Elements::Exception() << e.what() << ": " << e.code().message();
  }
}

void PythonInterpreter::captureOutput() {
  Pyston::GILLocker locker;
  PySys_SetObject("stdout", py::object(boost::ref(m_out_wrapper)).ptr());
  PySys_SetObject("stderr", py::object(boost::ref(m_err_wrapper)).ptr());
}

void PythonInterpreter::setupContext(boost::python::object config) {
  Pyston::GILLocker locker;
  try {
    if (config) {
      m_measurement_config = config;
    } else {
      m_measurement_config = py::import("sourcextractor.config.measurement_config").attr("global_measurement_config");
    }
  } catch (const py::error_already_set& e) {
    throw Pyston::Exception().log(log4cpp::Priority::ERROR, logger);
  }
}

std::map<int, PyMeasurementImage> PythonInterpreter::getMeasurementImages() {
  Pyston::GILLocker locker;

  try {
    py::dict   images  = py::extract<py::dict>(m_measurement_config.attr("measurement_images"));
    py::list   ids     = images.keys();
    std::map<int, PyMeasurementImage> result{};
    for (int i = 0; i < py::len(ids); ++i) {
      int                id = py::extract<int>(ids[i]);
      PyMeasurementImage im = py::extract<PyMeasurementImage>(images[ids[i]]);
      result.emplace(std::make_pair(id, im));
    }
    return result;
  } catch (const py::error_already_set& e) {
    throw Pyston::Exception().log(log4cpp::Priority::ERROR, logger);
  }
}

std::map<int, PyAperture> PythonInterpreter::getApertures() {
  Pyston::GILLocker locker;

  try {
    py::dict   apertures = py::extract<py::dict>(m_measurement_config.attr("apertures_for_image"));
    py::list   ids       = apertures.keys();
    std::map<int, PyAperture> result;
    for (int i = 0; i < py::len(ids); ++i) {
      int        id = py::extract<int>(ids[i]);
      PyAperture ap = py::extract<PyAperture>(apertures[ids[i]]);
      result.emplace(std::make_pair(id, ap));
    }
    return result;
  } catch (const py::error_already_set& e) {
    throw Pyston::Exception().log(log4cpp::Priority::ERROR, logger);
  }
}

std::vector<std::pair<std::string, std::vector<int>>> PythonInterpreter::getModelFittingOutputColumns() {
  Pyston::GILLocker locker;

  try {
    py::list   output  = py::extract<py::list>(m_measurement_config.attr("model_fitting_parameter_columns"));
    std::vector<std::pair<std::string, std::vector<int>>> result;
    for (int i = 0; i < py::len(output); ++i) {
      py::tuple   t            = py::extract<py::tuple>(output[i]);
      std::string name         = py::extract<std::string>(t[0]);
      auto        extract_list = py::extract<py::list>(t[1]);

      std::vector<int> ids{};
      if (extract_list.check()) {
        py::list cs = extract_list;
        for (int j = 0; j < py::len(cs); ++j) {
          int c = py::extract<int>(cs[j].attr("id"));
          ids.push_back(c);
        }
      } else {
        int c = py::extract<int>(t[1]);
        ids.push_back(c);
      }
      result.emplace_back(name, std::move(ids));
    }
    return result;
  } catch (const py::error_already_set& e) {
    throw Pyston::Exception().log(log4cpp::Priority::ERROR, logger);
  }
}

std::map<std::string, std::vector<int>> PythonInterpreter::getApertureOutputColumns() {
  Pyston::GILLocker locker;

  try {
    py::list   output  = py::extract<py::list>(m_measurement_config.attr("aperture_columns"));
    std::map<std::string, std::vector<int>> result;
    for (int i = 0; i < py::len(output); ++i) {
      py::tuple   t            = py::extract<py::tuple>(output[i]);
      std::string name         = py::extract<std::string>(t[0]);
      auto        extract_list = py::extract<py::list>(t[1]);

      if (extract_list.check()) {
        py::list cs = extract_list;
        for (int j = 0; j < py::len(cs); ++j) {
          int c = py::extract<int>(cs[j].attr("id"));
          result[name].push_back(c);
        }
      } else {
        int c = py::extract<int>(t[1]);
        result[name].push_back(c);
      }
    }
    return result;
  } catch (const py::error_already_set& e) {
    throw Pyston::Exception().log(log4cpp::Priority::ERROR, logger);
  }
}

std::map<int, boost::python::object> PythonInterpreter::getMapFromDict(const char* object_name,
                                                                       const char* dict_name) {
  Pyston::GILLocker locker;

  try {
    py::object obj        = m_measurement_config.attr(object_name);
    py::dict   parameters = py::extract<py::dict>(obj.attr(dict_name));
    py::list   ids        = parameters.keys();

    std::map<int, boost::python::object> result;
    for (int i = 0; i < py::len(ids); ++i) {
      int  id  = py::extract<int>(ids[i]);
      auto par = parameters[ids[i]];
      result.emplace(std::make_pair(id, par));
    }
    return result;
  } catch (const py::error_already_set& e) {
    throw Pyston::Exception().log(log4cpp::Priority::ERROR, logger);
  }
}

std::map<int, boost::python::object> PythonInterpreter::getConstantParameters() {
  return getMapFromDict("model_fitting", "constant_parameter_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getFreeParameters() {
  return getMapFromDict("model_fitting", "free_parameter_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getDependentParameters() {
  return getMapFromDict("model_fitting", "dependent_parameter_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getPriors() {
  return getMapFromDict("model_fitting", "prior_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getConstantModels() {
  return getMapFromDict("model_fitting", "constant_model_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getPointSourceModels() {
  return getMapFromDict("model_fitting", "point_source_model_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getSersicModels() {
  return getMapFromDict("model_fitting", "sersic_model_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getExponentialModels() {
  return getMapFromDict("model_fitting", "exponential_model_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getDeVaucouleursModels() {
  return getMapFromDict("model_fitting", "de_vaucouleurs_model_dict");
}

std::map<int, boost::python::object> PythonInterpreter::getOnnxModels() {
  return getMapFromDict("model_fitting", "onnx_model_dict");
}

std::map<int, std::vector<int>> PythonInterpreter::getFrameModelsMap() {
  Pyston::GILLocker locker;
  try {
    std::map<int, std::vector<int>> result{};
    py::object model_fitting = m_measurement_config.attr("model_fitting");
    py::dict   frame_dict    = py::extract<py::dict>(model_fitting.attr("frame_models_dict"));
    py::list   frame_ids     = frame_dict.keys();
    for (int i = 0; i < py::len(frame_ids); ++i) {
      int      frame_id  = py::extract<int>(frame_ids[i]);
      py::list model_ids = py::extract<py::list>(frame_dict[frame_ids[i]]);
      for (int j = 0; j < py::len(model_ids); ++j) {
        int model_id = py::extract<int>(model_ids[j]);
        result[frame_id].push_back(model_id);
      }
    }
    return result;
  } catch (const py::error_already_set& e) {
    throw Pyston::Exception().log(log4cpp::Priority::ERROR, logger);
  }
}

std::map<std::string, boost::python::object> PythonInterpreter::getModelFittingParams() {
  Pyston::GILLocker locker;

  py::object model_fitting = m_measurement_config.attr("model_fitting");
  py::dict   parameters    = py::extract<py::dict>(model_fitting.attr("params_dict"));
  py::list   ids           = parameters.keys();

  std::map<std::string, boost::python::object> result;
  for (int i = 0; i < py::len(ids); ++i) {
    std::string id  = py::extract<std::string>(ids[i]);
    auto        par = parameters[ids[i]];
    result.emplace(std::make_pair(id, par));
  }
  return result;
}

}  // namespace SourceXtractor
