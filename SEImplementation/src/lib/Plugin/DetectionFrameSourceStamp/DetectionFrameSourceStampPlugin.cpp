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
 * DetectionFrameSourceStampPlugin.cpp
 *
 *  Created on: Aug 5, 2016
 *      Author: mschefer
 */

#include "SEImplementation/Plugin/DetectionFrameSourceStamp/DetectionFrameSourceStampPlugin.h"
#include "NdArray/Borrowed.h"
#include "SEFramework/Plugin/StaticPlugin.h"
#include "SEImplementation/Plugin/DetectionFrameSourceStamp/DetectionFrameSourceStamp.h"
#include "SEImplementation/Plugin/DetectionFrameSourceStamp/DetectionFrameSourceStampTaskFactory.h"

namespace SourceXtractor {

using Euclid::NdArray::BorrowedRange;
using Euclid::NdArray::NdArray;

static StaticPlugin<DetectionFrameSourceStampPlugin> detection_frame_source_stamp_plugin;

struct StampConverter {
  using DetectionVectorImage = DetectionFrameSourceStamp::DetectionVectorImage;
  using PixelType            = DetectionVectorImage::PixelType;

  std::function<const DetectionVectorImage&(const DetectionFrameSourceStamp&)> m_getter;

  NdArray<PixelType> operator()(const DetectionFrameSourceStamp& property) const {
    auto&               stamp = m_getter(property);
    auto&               data  = const_cast<std::vector<PixelType>&>(stamp.getData());
    std::vector<size_t> shape{static_cast<size_t>(stamp.getHeight()), static_cast<size_t>(stamp.getWidth())};
    return NdArray<DetectionImage::PixelType>(shape, BorrowedRange<PixelType>{data.data(), data.size()});
  }
};

void DetectionFrameSourceStampPlugin::registerPlugin(PluginAPI& plugin_api) {
  using PixelType = DetectionImage::PixelType;

  plugin_api.getOutputRegistry().registerColumnConverter<DetectionFrameSourceStamp, NdArray<PixelType>>(
      "detection_stamp", StampConverter{&DetectionFrameSourceStamp::getStamp});
  plugin_api.getOutputRegistry().registerColumnConverter<DetectionFrameSourceStamp, NdArray<PixelType>>(
      "detection_filtered_stamp", StampConverter{&DetectionFrameSourceStamp::getFilteredStamp});
  plugin_api.getOutputRegistry().registerColumnConverter<DetectionFrameSourceStamp, NdArray<PixelType>>(
      "detection_variance_stamp", StampConverter{&DetectionFrameSourceStamp::getVarianceStamp});
  plugin_api.getOutputRegistry().registerColumnConverter<DetectionFrameSourceStamp, NdArray<PixelType>>(
      "detection_thresholded_stamp", StampConverter{&DetectionFrameSourceStamp::getThresholdedStamp});

  plugin_api.getTaskFactoryRegistry()
      .registerTaskFactory<DetectionFrameSourceStampTaskFactory, DetectionFrameSourceStamp>();
}

std::string DetectionFrameSourceStampPlugin::getIdString() const {
  return "DetectionFrameSourceStamp";
}

}  // namespace SourceXtractor
