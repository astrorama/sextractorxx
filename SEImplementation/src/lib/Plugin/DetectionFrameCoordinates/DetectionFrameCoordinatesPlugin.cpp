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

#include "SEFramework/Plugin/StaticPlugin.h"

#include "SEImplementation/Plugin/DetectionFrameCoordinates/DetectionFrameCoordinates.h"
#include "SEImplementation/Plugin/DetectionFrameCoordinates/DetectionFrameCoordinatesTaskFactory.h"
#include "SEImplementation/Plugin/DetectionFrameCoordinates/DetectionFrameCoordinatesPlugin.h"
#include "SEImplementation/Image/ImageInterfaceTraits.h"

namespace SourceXtractor {

static StaticPlugin<DetectionFrameCoordinatesPlugin> detection_frame_coordinates_plugin;

void DetectionFrameCoordinatesPlugin::registerPlugin(PluginAPI& plugin_api) {
  plugin_api.getTaskFactoryRegistry().registerTaskFactory<DetectionFrameCoordinatesTaskFactory, DetectionFrameCoordinates>();
}

std::string DetectionFrameCoordinatesPlugin::getIdString() const {
  return "DetectionFrameCoordinates";
}

}


