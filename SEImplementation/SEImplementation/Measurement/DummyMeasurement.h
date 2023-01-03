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
 * DummyMeasurement.h
 *
 *  Created on: May 17, 2018
 *      Author: mschefer
 */

#ifndef _SEIMPLEMENTATION_MEASUREMENT_DUMMYMEASUREMENT_H_
#define _SEIMPLEMENTATION_MEASUREMENT_DUMMYMEASUREMENT_H_

#include "SEFramework/Pipeline/Measurement.h"

namespace SourceXtractor {

class DummyMeasurement : public Measurement {
public:

  void receiveSource(std::unique_ptr<SourceGroupInterface> source_group) override {
    sendSource(std::move(source_group));

  }
  void receiveProcessSignal(const ProcessSourcesEvent& event) override {
    sendProcessSignal(event);
  }

  void startThreads() override {};
  void stopThreads() override {};
  void synchronizeThreads() override {};
  void cancel() override {};
};

}


#endif /* SEIMPLEMENTATION_SEIMPLEMENTATION_MEASUREMENT_DUMMYMEASUREMENT_H_ */
