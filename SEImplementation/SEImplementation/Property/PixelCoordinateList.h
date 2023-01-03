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
 * @file PixelCoordinateList.h
 * @author nikoapos
 */

#ifndef _SEIMPLEMENTATION_PIXELCOORDINATELIST_H
#define _SEIMPLEMENTATION_PIXELCOORDINATELIST_H

#include <algorithm>
#include "SEUtils/PixelCoordinate.h"
#include "SEFramework/Property/Property.h"

namespace SourceXtractor {

class PixelCoordinateList : public Property {

public:

  explicit PixelCoordinateList(std::vector<PixelCoordinate> coordinate_list)
      : m_coordinate_list(std::move(coordinate_list)) {
  }

  PixelCoordinateList(const PixelCoordinateList&) = default;
  PixelCoordinateList(PixelCoordinateList&&) = default;

  virtual ~PixelCoordinateList() = default;

  const std::vector<PixelCoordinate>& getCoordinateList() const {
    return m_coordinate_list;
  }

  bool contains(const PixelCoordinate& coord) const {
    return std::find(m_coordinate_list.begin(), m_coordinate_list.end(), coord) != m_coordinate_list.end();
  }

private:

  std::vector<PixelCoordinate> m_coordinate_list;

}; /* End of PixelCoordinateList class */

} /* namespace SourceXtractor */

#endif /* _SEIMPLEMENTATION_PIXELCOORDINATELIST_H */

