/*
 * BackgroundAnalyzerFactory.cpp
 *
 *  Created on: Mar 29, 2017
 *      Author: mschefer
 */


#include "SEImplementation/Background/BackgroundLevelAnalyzerFactory.h"

#include "SEImplementation/Background/SimpleBackgroundLevelAnalyzer.h"
#include "SEImplementation/Background/SE2BackgroundLevelAnalyzer.h"

namespace SExtractor {

std::shared_ptr<BackgroundAnalyzer> BackgroundLevelAnalyzerFactory::createBackgroundAnalyzer() const {
  auto background_level =  std::make_shared<SE2BackgroundLevelAnalyzer>();
  background_level->setParameters(m_cell_size, m_smoothing_box);
  return background_level;
}

void BackgroundLevelAnalyzerFactory::reportConfigDependencies(Euclid::Configuration::ConfigManager& manager) const {
  manager.registerConfiguration<SE2BackgroundConfig>();
}

void BackgroundLevelAnalyzerFactory::configure(Euclid::Configuration::ConfigManager& manager) {
  auto se2background_config = manager.getConfiguration<SE2BackgroundConfig>();
  m_cell_size = se2background_config.getCellSize();
  m_smoothing_box = se2background_config.getSmoothingBox();
}

}
