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
 * Measurement.h
 *
 *  Created on: May 17, 2018
 *      Author: mschefer
 */

#ifndef _SEFRAMEWORK_PIPELINE_MEASUREMENT_H_
#define _SEFRAMEWORK_PIPELINE_MEASUREMENT_H_


#include "SEFramework/Pipeline/PipelineStage.h"
#include "SEFramework/Source/SourceGroupInterface.h"
#include "SEFramework/Task/TaskProvider.h"

namespace SourceXtractor {

class Measurement : public PipelineReceiver<SourceGroupInterface>, public PipelineEmitter<SourceGroupInterface> {
public:

  ~Measurement() override = default;

  virtual void startThreads() = 0;
  virtual void stopThreads() = 0;
  virtual void synchronizeThreads() = 0;
  virtual void cancel() = 0;
};

}


#endif /* _SEFRAMEWORK_PIPELINE_MEASUREMENT_H_ */
