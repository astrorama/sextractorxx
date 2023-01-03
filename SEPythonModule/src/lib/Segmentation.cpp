/**
 * Copyright © 2022 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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

#include "SEPythonModule/Segmentation.h"
#include "Pyston/GIL.h"
#include "SEFramework/Pipeline/SourceGrouping.h"
#include "SEImplementation/Configuration/DetectionFrameConfig.h"
#include "SEImplementation/Segmentation/SegmentationFactory.h"
#include "SEPythonModule/PipelineReceiver.h"
#include <boost/python/extract.hpp>
#include <boost/range/algorithm.hpp>

namespace SourceXPy {

namespace py = boost::python;

using SourceXtractor::DetectionFrameConfig;
using SourceXtractor::DetectionImageFrame;
using SourceXtractor::ProcessSourcesEvent;

Segmentation::Segmentation(ContextPtr context) : m_context(std::move(context)) {
  m_segmentation = m_context->m_segmentation_factory->createSegmentation();
}

std::string Segmentation::repr() const {
  return "Segmentation";
}

void Segmentation::setNextStage(const py::object& callback) {
  py::extract<std::shared_ptr<SourceReceiverIfce>> cpp_ifce(callback);
  if (cpp_ifce.check()) {
    m_next_stage = cpp_ifce();
  } else {
    m_next_stage = std::make_shared<PipelineSourceReceiver>(callback, m_context);
  }
  m_segmentation->setNextStage(m_next_stage);
}

void Segmentation::call() const {
  Pyston::SaveThread save_thread;
  const auto&        detection_config = m_context->m_config_manager->getConfiguration<DetectionFrameConfig>();
  const auto&        frames           = detection_config.getDetectionFrames();
  boost::for_each(frames,
                  [this](const std::shared_ptr<DetectionImageFrame>& frame) { m_segmentation->processFrame(frame); });
  m_next_stage->receiveProcessSignal(ProcessSourcesEvent(std::make_shared<AllFramesDone>()));
}

}  // namespace SourceXPy
