/** Copyright © 2022 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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


#include "SEImplementation/Property/PixelCoordinateList.h"
#include "SEImplementation/Property/SourceId.h"
#include "SEImplementation/Plugin/PixelCentroid/PixelCentroid.h"
#include "SEImplementation/Plugin/AssocMode/AssocMode.h"
#include "SEImplementation/Plugin/WorldCentroid/WorldCentroid.h"

#include "SEImplementation/Segmentation/AssocSegmentation.h"

namespace SourceXtractor {

void AssocSegmentation::labelImage(Segmentation::LabellingListener& listener, std::shared_ptr<const DetectionImageFrame> ) {
  for (auto source_coordinate : m_source_list) {
    auto source = m_source_factory->createSource();
    source->setProperty<SourceId>();
    source->setProperty<WorldCentroid>(source_coordinate.world_coord.m_alpha, source_coordinate.world_coord.m_delta);
    source->setProperty<AssocMode>(true, source_coordinate.assoc_columns,
        source_coordinate.source_pixel_width, source_coordinate.source_pixel_height, source_coordinate.group_id);

    listener.publishSource(std::move(source));
  }
}

}
