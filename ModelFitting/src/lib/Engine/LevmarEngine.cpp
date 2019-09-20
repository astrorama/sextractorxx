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
/** 
 * @file LevmarEngine.cpp
 * @date August 17, 2015
 * @author Nikolaos Apostolakos
 */

#include <cmath>
#include <mutex>

#ifndef WITHOUT_LEVMAR
#include <levmar.h>
#endif

#include "ModelFitting/Engine/LevmarEngine.h"

#include <iostream>
#include <ElementsKernel/Exception.h>

namespace ModelFitting {

LevmarEngine::LevmarEngine(size_t itmax, double tau, double epsilon1,
               double epsilon2, double epsilon3, double delta)
      : m_itmax{itmax}, m_opts{tau, epsilon1, epsilon2, epsilon3, delta} { }
      
LevmarEngine::~LevmarEngine() = default;

// The Levmar library seems to have some problems with multithreading, this mutex is used to ensure only one thread
// in levmar
namespace {
  std::mutex levmar_mutex;
}

LeastSquareSummary LevmarEngine::solveProblem(EngineParameterManager& parameter_manager,
                                              ResidualEstimator& residual_estimator) {

#ifdef WITHOUT_LEVMAR
  throw Elements::Exception() << "Binary compiled without Levmar! No model fitting possible";
#else
  // Create a tuple which keeps the references to the given manager and estimator
  auto adata = std::tie(parameter_manager, residual_estimator);

  // The function which is called by the levmar loop
  auto levmar_res_func = [](double *p, double *hx, int, int, void *extra) {
    levmar_mutex.unlock();

    auto* extra_ptr = (decltype(adata)*)extra;
    EngineParameterManager& pm = std::get<0>(*extra_ptr);
    pm.updateEngineValues(p);
    ResidualEstimator& re = std::get<1>(*extra_ptr);
    re.populateResiduals(hx);

    levmar_mutex.lock();
  };

  // Create the vector which will be used for keeping the parameter values
  // and initialize it to the current values of the parameters
  std::vector<double> param_values (parameter_manager.numberOfParameters());
  parameter_manager.getEngineValues(param_values.begin());

  // Create a vector for getting the information of the minimization
  std::array<double, 10> info;

  std::vector<double> covariance_matrix (parameter_manager.numberOfParameters() * parameter_manager.numberOfParameters());

  levmar_mutex.lock();
  // Call the levmar library
  auto res = dlevmar_dif(levmar_res_func, // The function called from the levmar algorithm
                         param_values.data(), // The pointer where the parameter values are
                         NULL, // We don't use any measurement vector
                         parameter_manager.numberOfParameters(), // The number of free parameters
                         residual_estimator.numberOfResiduals(), // The number of residuals
                         m_itmax, // The maximum number of iterations
                         m_opts.data(), // The minimization options
                         info.data(), // Where the information of the minimization is stored
                         NULL, // Working memory is allocated internally
                         covariance_matrix.data(),
                         &adata // No additional data needed
                        );
  levmar_mutex.unlock();
  
  // Create and return the summary object
  LeastSquareSummary summary {};

  auto converted_covariance_matrix = parameter_manager.convertCovarianceMatrixToWorldSpace(covariance_matrix);
  for (unsigned int i=0; i<parameter_manager.numberOfParameters(); i++) {
    summary.parameter_sigmas.push_back(sqrt(converted_covariance_matrix[i*(parameter_manager.numberOfParameters()+1)]));
  }

  summary.success_flag = (res != -1);
  summary.iteration_no = info[5];
  summary.underlying_framework_info = info;
  return summary;
#endif
}

} // end of namespace ModelFitting
