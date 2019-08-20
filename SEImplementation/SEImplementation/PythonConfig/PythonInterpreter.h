/* 
 * @file PythonInterpreter.h
 * @author Nikolaos Apostolakos <nikoapos@gmail.com>
 */

#ifndef _SEIMPLEMENTATION_PYTHONINTERPRETER_H
#define _SEIMPLEMENTATION_PYTHONINTERPRETER_H

#include <string>
#include <map>
#include <vector>
#include <SEFramework/CoordinateSystem/CoordinateSystem.h>
#include <SEImplementation/PythonConfig/PyMeasurementImage.h>
#include <SEImplementation/PythonConfig/PyAperture.h>
#include <SEImplementation/PythonConfig/PyOutputWrapper.h>

namespace SExtractor {

class PythonInterpreter {
  
public:
  
  static PythonInterpreter& getSingleton();
  
  void runCode(const std::string& code);
  
  void runFile(const std::string& filename, const std::vector<std::string>& argv);

  virtual ~PythonInterpreter() = default;
  
  std::map<int, PyMeasurementImage> getMeasurementImages();

  std::map<int, PyAperture> getApertures();

  std::vector<std::pair<std::string, std::vector<int>>> getModelFittingOutputColumns();

  std::map<std::string, std::vector<int>> getApertureOutputColumns();

  std::map<int, boost::python::object> getConstantParameters();
  
  std::map<int, boost::python::object> getFreeParameters();
  
  std::map<int, boost::python::object> getDependentParameters();
  
  std::map<int, boost::python::object> getPriors();
  
  std::map<int, boost::python::object> getConstantModels();

  std::map<int, boost::python::object> getPointSourceModels();
  
  std::map<int, boost::python::object> getSersicModels();
  
  std::map<int, boost::python::object> getExponentialModels();
  
  std::map<int, boost::python::object> getDeVaucouleursModels();
  
  std::map<int, std::vector<int>> getFrameModelsMap();

  std::map<std::string, boost::python::object> getModelFittingParams();

  std::vector<boost::python::object> getMeasurementGroups();

  void setCoordinateSystem(std::shared_ptr<CoordinateSystem> coordinate_system);

private:
  
  PythonInterpreter();
  PyOutputWrapper m_out_wrapper, m_err_wrapper;
};

} // end of namespace SExtractor

#endif // _SEIMPLEMENTATION_PYTHONINTERPRETER_H

