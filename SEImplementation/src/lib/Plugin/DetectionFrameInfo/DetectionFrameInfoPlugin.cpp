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

#include "SEFramework/Plugin/StaticPlugin.h"

#include "SEImplementation/Image/ImageInterfaceTraits.h"
#include "SEImplementation/Plugin/DetectionFrameInfo/DetectionFrameInfo.h"
#include "SEImplementation/Plugin/DetectionFrameInfo/DetectionFrameInfoPlugin.h"
#include "SEImplementation/Plugin/DetectionFrameInfo/DetectionFrameInfoTaskFactory.h"

namespace SourceXtractor {

static StaticPlugin<DetectionFrameInfoPlugin> detection_frame_info_plugin;

void DetectionFrameInfoPlugin::registerPlugin(PluginAPI& plugin_api) {
  plugin_api.getTaskFactoryRegistry().registerTaskFactory<DetectionFrameInfoTaskFactory, DetectionFrameInfo>();

  plugin_api.getOutputRegistry().registerColumnConverter<DetectionFrameInfo, int>(
      "detection_frame_hdu", [](const DetectionFrameInfo& frame_info) { return frame_info.getHduIndex(); });
}

std::string DetectionFrameInfoPlugin::getIdString() const {
  return "DetectionFrameInfo";
}

}  // namespace SourceXtractor
