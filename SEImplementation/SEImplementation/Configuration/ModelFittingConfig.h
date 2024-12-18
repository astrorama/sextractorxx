/** Copyright © 2019 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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
 * @file ModelFittingConfig.h
 * @author Nikolaos Apostolakos <nikoapos@gmail.com>
 */

#ifndef _SEIMPLEMENTATION_CONFIGURATION_MODELFITTINGCONFIG_H
#define _SEIMPLEMENTATION_CONFIGURATION_MODELFITTINGCONFIG_H

#include <vector>

#include <SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingParameter.h>
#include <SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingModel.h>
#include <SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingFrame.h>
#include <SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingPrior.h>

#include <SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingIterativeTask.h>

#include <Configuration/Configuration.h>

namespace SourceXtractor {

class ModelFittingConfig : public Euclid::Configuration::Configuration {
  
public:
  
  explicit ModelFittingConfig(long manager_id);

  ~ModelFittingConfig();
  
  void initialize(const UserValues& args) override;
  
  const std::map<int, std::shared_ptr<FlexibleModelFittingParameter>>& getParameters() const;
  
  const std::map<int, std::shared_ptr<FlexibleModelFittingModel>>& getModels() const;
  const std::vector<std::shared_ptr<FlexibleModelFittingFrame>>& getFrames() const;
  
  const std::map<int, std::shared_ptr<FlexibleModelFittingPrior>>& getPriors() const;
  
  const std::vector<std::pair<std::string, std::vector<int>>>& getOutputs() const;

  std::string getLeastSquaresEngine() const {
    return m_least_squares_engine;
  }

  unsigned int getMaxIterations() const { return m_max_iterations; }
  double getModifiedChiSquaredScale() const { return m_modified_chi_squared_scale; }
  bool getUseIterativeFitting() const { return m_use_iterative_fitting; }
  int getMetaIterations() const { return m_meta_iterations; }
  double getDeblendFactor() const { return m_deblend_factor; }
  double getMetaIterationStop() const { return m_meta_iteration_stop; }
  FlexibleModelFittingIterativeTask::WindowType getWindowType() const { return m_window_type; }
  double getEllipseScale() const { return m_ellipse_scale; }

private:
  std::string m_least_squares_engine;
  unsigned int m_max_iterations {0};
  double m_modified_chi_squared_scale {10.};
  bool m_use_iterative_fitting { true };
  int m_meta_iterations { 3 };
  double m_deblend_factor { 1.0 };
  double m_meta_iteration_stop { 0.0001 };
  FlexibleModelFittingIterativeTask::WindowType m_window_type
      { FlexibleModelFittingIterativeTask::WindowType::RECTANGLE };
  double m_ellipse_scale { 3.0 };
  
  std::map<int, std::shared_ptr<FlexibleModelFittingParameter>> m_parameters;
  std::map<int, std::shared_ptr<FlexibleModelFittingModel>> m_models;
  std::vector<std::shared_ptr<FlexibleModelFittingFrame>> m_frames;
  std::map<int, std::shared_ptr<FlexibleModelFittingPrior>> m_priors;
  std::vector<std::pair<std::string, std::vector<int>>> m_outputs;

  /// The initialization code calls Python. It gets wrapped here so we can catch any
  /// potential error and translate into a Elements exception
  void initializeInner();
};

}

#endif // _SEIMPLEMENTATION_CONFIGURATION_MODELFITTINGCONFIG_H

