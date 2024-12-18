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
 * FlexibleModelFittingTaskFactory.h
 *
 *  Created on: Sep 17, 2018
 *      Author: mschefer
 */

#ifndef _SEIMPLEMENTATION_PLUGIN_FLEXIBLEMODELFITTING_FLEXIBLEMODELFITTINGTASKFACTORY_H_
#define _SEIMPLEMENTATION_PLUGIN_FLEXIBLEMODELFITTING_FLEXIBLEMODELFITTINGTASKFACTORY_H_

#include "SEFramework/Task/TaskFactory.h"

#include "SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingIterativeTask.h"

namespace SourceXtractor {

class FlexibleModelFittingTaskFactory : public TaskFactory {
public:
  FlexibleModelFittingTaskFactory() {}

  /// Destructor
  virtual ~FlexibleModelFittingTaskFactory() = default;

  // TaskFactory implementation
  std::shared_ptr<Task> createTask(const PropertyId& property_id) const override;

  void reportConfigDependencies(Euclid::Configuration::ConfigManager& manager) const override;
  void configure(Euclid::Configuration::ConfigManager& manager) override;
  void registerPropertyInstances(OutputRegistry&) override;

private:
  std::string m_least_squares_engine;
  unsigned int m_max_iterations {0};
  double m_modified_chi_squared_scale {10};
  bool m_use_iterative_fitting { true };
  int m_meta_iterations { 3 };
  double m_deblend_factor { 1.0 };
  double m_meta_iteration_stop { 0.0001 };
  FlexibleModelFittingIterativeTask::WindowType m_window_type { FlexibleModelFittingIterativeTask::WindowType::RECTANGLE };
  double m_ellipse_scale { 3.0 };

  std::vector<std::shared_ptr<FlexibleModelFittingParameter>> m_parameters;
  std::vector<std::shared_ptr<FlexibleModelFittingFrame>> m_frames;
  std::vector<std::pair<std::string, std::vector<int>>> m_outputs;
  std::vector<std::shared_ptr<FlexibleModelFittingPrior>> m_priors;

  double m_scale_factor {1.0};
  double m_max_fit_size {100};
};

}




#endif /* _SEIMPLEMENTATION_PLUGIN_FLEXIBLEMODELFITTING_FLEXIBLEMODELFITTINGTASKFACTORY_H_ */
