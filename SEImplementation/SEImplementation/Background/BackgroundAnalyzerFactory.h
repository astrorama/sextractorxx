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
 * BackgroundAnalyzerFactory.h
 *
 *  Created on: Mar 29, 2017
 *      Author: mschefer
 */

#ifndef _SEIMPLEMENTATION_BACKGROUND_BACKGROUNDANALYZERFACTORY_H_
#define _SEIMPLEMENTATION_BACKGROUND_BACKGROUNDANALYZERFACTORY_H_

#include "SEImplementation/Configuration/SE2BackgroundConfig.h"
#include "SEImplementation/Configuration/WeightImageConfig.h"

#include "SEFramework/Configuration/Configurable.h"
#include "SEFramework/Background/BackgroundAnalyzer.h"

namespace SExtractor {

class BackgroundAnalyzerFactory  : public Configurable {
public:
  /// Destructor
  virtual ~BackgroundAnalyzerFactory() = default;

  std::shared_ptr<BackgroundAnalyzer> createBackgroundAnalyzer() const;
  std::shared_ptr<BackgroundAnalyzer> createBackgroundAnalyzer(WeightImageConfig::WeightType weight_type) const;

  // Implementation of the Configurable interface
  void configure(Euclid::Configuration::ConfigManager& manager) override;
  void reportConfigDependencies(Euclid::Configuration::ConfigManager& manager) const override;

private:
  std::string m_cell_size;
  std::string m_smoothing_box;
  WeightImageConfig::WeightType m_weight_type;
};

}



#endif /* _SEIMPLEMENTATION_BACKGROUND_BACKGROUNDANALYZERFACTORY_H_ */
