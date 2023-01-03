/*
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
 * DetectionFramePixelValuesPlugin.cpp
 *
 *  Created on: Aug 5, 2016
 *      Author: mschefer
 */

#include "SEImplementation/Plugin/DetectionFramePixelValues/DetectionFramePixelValuesPlugin.h"
#include "SEFramework/Plugin/StaticPlugin.h"
#include "SEImplementation/Plugin/DetectionFramePixelValues/DetectionFramePixelValues.h"
#include "SEImplementation/Plugin/DetectionFramePixelValues/DetectionFramePixelValuesTaskFactory.h"

namespace SourceXtractor {

static StaticPlugin<DetectionFramePixelValuesPlugin> detection_frame_pixel_values_plugin;

void DetectionFramePixelValuesPlugin::registerPlugin(PluginAPI& plugin_api) {
  auto& output_registry = plugin_api.getOutputRegistry();
  output_registry.registerColumnConverter<DetectionFramePixelValues, std::vector<DetectionImage::PixelType>>(
      "detection_pixel_values", [](const DetectionFramePixelValues& pixels) { return pixels.getValues(); });
  plugin_api.getTaskFactoryRegistry()
      .registerTaskFactory<DetectionFramePixelValuesTaskFactory, DetectionFramePixelValues>();
}

std::string DetectionFramePixelValuesPlugin::getIdString() const {
  return "";
}

}  // namespace SourceXtractor
