/* 
 * @file ModelFittingConfig.cpp
 * @author Nikolaos Apostolakos <nikoapos@gmail.com>
 */

#include <ElementsKernel/Logging.h>
#include <SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingParameter.h>
#include <SEImplementation/PythonConfig/ObjectInfo.h>
#include <SEImplementation/Configuration/PythonConfig.h>
#include <SEImplementation/Configuration/ModelFittingConfig.h>
#include <SEUtils/Python.h>

using namespace Euclid::Configuration;
namespace py = boost::python;

static Elements::Logging logger = Elements::Logging::getLogger("ModelFittingConfig");

namespace SExtractor {

/**
 * Wrap py::extract *and* the call so Python errors can be properly translated and logged
 * @tparam R
 *  Return type
 * @tparam T
 *  Variadic template for any arbitrary number of arguments
 * @param func
 *  Python function to be called
 * @param args
 *  Arguments for the Python function
 * @return
 *  Whatever the Python function returns
 * @throw
 *  Elements::Exception if either the call or the extract throw a Python exception
 */
template <typename R, typename ...T>
R py_call_wrapper(py::object func, T... args) {
  try {
    return py::extract<R>(func(args...));
  }
  catch (const py::error_already_set &e) {
    throw pyToElementsException(logger);
  }
}

ModelFittingConfig::ModelFittingConfig(long manager_id) : Configuration(manager_id) {
  declareDependency<PythonConfig>();
}

void ModelFittingConfig::initialize(const UserValues&) {
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getConstantParameters()) {
    py::object py_value_func = p.second.attr("get_value")();
    auto value_func = [py_value_func] (const SourceInterface& o) -> double {
      ObjectInfo oi {o};
      return py_call_wrapper<double>(py_value_func, oi);
    };
    m_parameters[p.first] = std::make_shared<FlexibleModelFittingConstantParameter>(
                                                           p.first, value_func);
  }
  
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getFreeParameters()) {
    py::object py_init_value_func = p.second.attr("get_init_value")();
    auto init_value_func = [py_init_value_func] (const SourceInterface& o) -> double {
      ObjectInfo oi {o};
      return py_call_wrapper<double>(py_init_value_func, oi);
    };
    py::object py_range_obj = p.second.attr("get_range")();
    py::object py_range_func = py_range_obj.attr("get_limits")();
    auto range_func = [py_range_func] (double init, const SourceInterface& o) -> std::pair<double, double> {
      ObjectInfo oi {o};
      py::tuple range = py_call_wrapper<py::tuple>(py_range_func, init, oi);
      double low = py::extract<double>(range[0]);
      double high = py::extract<double>(range[1]);
      return {low, high};
    };
    bool is_exponential = py::extract<int>(py_range_obj.attr("get_type")().attr("value")) == 2;
    m_parameters[p.first] = std::make_shared<FlexibleModelFittingFreeParameter>(
                          p.first, init_value_func, range_func, is_exponential);
  }
  
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getDependentParameters()) {
    py::object py_func = p.second.attr("func");
    std::vector<std::shared_ptr<FlexibleModelFittingParameter>> params {};
    py::list param_ids = py::extract<py::list>(p.second.attr("params"));
    for (int i = 0; i < py::len(param_ids); ++i) {
      int id = py::extract<int>(param_ids[i]);
      params.push_back(m_parameters[id]);
    }
    m_parameters[p.first] = std::make_shared<FlexibleModelFittingDependentParameter>(
                                                      p.first, py_func, params);
  }
  
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getPointSourceModels()) {
    int x_coord_id = py::extract<int>(p.second.attr("x_coord").attr("id"));
    int y_coord_id = py::extract<int>(p.second.attr("y_coord").attr("id"));
    int flux_id = py::extract<int>(p.second.attr("flux").attr("id"));
    m_models[p.first] = std::make_shared<FlexibleModelFittingPointModel>(
        m_parameters[x_coord_id], m_parameters[y_coord_id], m_parameters[flux_id]);
  }
  
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getSersicModels()) {
    int x_coord_id = py::extract<int>(p.second.attr("x_coord").attr("id"));
    int y_coord_id = py::extract<int>(p.second.attr("y_coord").attr("id"));
    int flux_id = py::extract<int>(p.second.attr("flux").attr("id"));
    int effective_radius_id = py::extract<int>(p.second.attr("effective_radius").attr("id"));
    int aspect_ratio_id = py::extract<int>(p.second.attr("aspect_ratio").attr("id"));
    int angle_id = py::extract<int>(p.second.attr("angle").attr("id"));
    int n_id = py::extract<int>(p.second.attr("n").attr("id"));
    m_models[p.first] = std::make_shared<FlexibleModelFittingSersicModel>(
        m_parameters[x_coord_id], m_parameters[y_coord_id], m_parameters[flux_id], m_parameters[n_id],
        m_parameters[effective_radius_id], m_parameters[aspect_ratio_id],
        m_parameters[angle_id]);
  }
  
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getExponentialModels()) {
    int x_coord_id = py::extract<int>(p.second.attr("x_coord").attr("id"));
    int y_coord_id = py::extract<int>(p.second.attr("y_coord").attr("id"));
    int flux_id = py::extract<int>(p.second.attr("flux").attr("id"));
    int effective_radius_id = py::extract<int>(p.second.attr("effective_radius").attr("id"));
    int aspect_ratio_id = py::extract<int>(p.second.attr("aspect_ratio").attr("id"));
    int angle_id = py::extract<int>(p.second.attr("angle").attr("id"));
    m_models[p.first] = std::make_shared<FlexibleModelFittingExponentialModel>(
        m_parameters[x_coord_id], m_parameters[y_coord_id], m_parameters[flux_id],
        m_parameters[effective_radius_id], m_parameters[aspect_ratio_id], m_parameters[angle_id]);
  }
  
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getDeVaucouleursModels()) {
    int x_coord_id = py::extract<int>(p.second.attr("x_coord").attr("id"));
    int y_coord_id = py::extract<int>(p.second.attr("y_coord").attr("id"));
    int flux_id = py::extract<int>(p.second.attr("flux").attr("id"));
    int effective_radius_id = py::extract<int>(p.second.attr("effective_radius").attr("id"));
    int aspect_ratio_id = py::extract<int>(p.second.attr("aspect_ratio").attr("id"));
    int angle_id = py::extract<int>(p.second.attr("angle").attr("id"));
    m_models[p.first] = std::make_shared<FlexibleModelFittingDevaucouleursModel>(
        m_parameters[x_coord_id], m_parameters[y_coord_id], m_parameters[flux_id],
        m_parameters[effective_radius_id], m_parameters[aspect_ratio_id], m_parameters[angle_id]);
  }
  
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getFrameModelsMap()) {
    std::vector<std::shared_ptr<FlexibleModelFittingModel>> model_list {};
    for (int x : p.second) {
      model_list.push_back(m_models[x]);
    }
    m_frames.push_back(std::make_shared<FlexibleModelFittingFrame>(p.first, model_list));
  }
  
  for (auto& p : getDependency<PythonConfig>().getInterpreter().getPriors()) {
    auto& prior = p.second;
    int param_id = py::extract<int>(prior.attr("param"));
    auto param = m_parameters[param_id];
    py::object py_value_func = prior.attr("value");
    auto value_func = [py_value_func] (const SourceInterface& o) -> double {
      ObjectInfo oi {o};
      return py_call_wrapper<double>(py_value_func, oi);
    };
    py::object py_sigma_func = prior.attr("sigma");
    auto sigma_func = [py_sigma_func] (const SourceInterface& o) -> double {
      ObjectInfo oi {o};
      return py_call_wrapper<double>(py_sigma_func, oi);
    };
    m_priors[p.first] = std::make_shared<FlexibleModelFittingPrior>(param, value_func, sigma_func);
  }
  
  m_outputs = getDependency<PythonConfig>().getInterpreter().getModelFittingOutputColumns();

  auto parameters = getDependency<PythonConfig>().getInterpreter().getModelFittingParams();
  m_max_iterations = py::extract<int>(parameters["max_iterations"]);
}

const std::map<int, std::shared_ptr<FlexibleModelFittingParameter>>& ModelFittingConfig::getParameters() const {
  return m_parameters;
}

const std::map<int, std::shared_ptr<FlexibleModelFittingModel>>& ModelFittingConfig::getModels() const {
  return m_models;
}

const std::vector<std::shared_ptr<FlexibleModelFittingFrame> >& ModelFittingConfig::getFrames() const {
  return m_frames;
}

const std::map<int, std::shared_ptr<FlexibleModelFittingPrior> >& ModelFittingConfig::getPriors() const {
  return m_priors;
}

const std::vector<std::pair<std::string, std::vector<int>>>& ModelFittingConfig::getOutputs() const {
  return m_outputs;
}

}
