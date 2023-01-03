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

#include "SEImplementation/Property/RegisterConverters.h"
#include "SEImplementation/Property/PixelCoordinateList.h"
#include "SEImplementation/Property/SourceId.h"

using Euclid::NdArray::NdArray;

namespace SourceXtractor {

void registerDefaultConverters(OutputRegistry& registry) {
  registry.registerColumnConverter<PixelCoordinateList, NdArray<int32_t>>(
      "pixel_coordinates", [](const PixelCoordinateList& coordinates) {
        const auto&      list = coordinates.getCoordinateList();
        NdArray<int32_t> coords({list.size(), 2});
        for (size_t i = 0; i < list.size(); ++i) {
          coords.at(i, 0) = list[i].m_x;
          coords.at(i, 1) = list[i].m_y;
        }
        return coords;
      });
}

}  // namespace SourceXtractor
