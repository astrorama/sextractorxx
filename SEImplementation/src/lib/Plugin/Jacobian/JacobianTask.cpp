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
/*
 * JacobianTask.cpp
 *
 *  Created on: Oct 08, 2018
 *      Author: Alejandro Alvarez Ayllon
 */

#include <iostream>

#include "SEImplementation/Plugin/PixelBoundaries/PixelBoundaries.h"
#include "SEImplementation/Plugin/DetectionFrameGroupStamp/DetectionFrameGroupStamp.h"
#include "SEImplementation/Plugin/DetectionFrameSourceStamp/DetectionFrameSourceStamp.h"
#include "SEImplementation/Plugin/DetectionFrameCoordinates/DetectionFrameCoordinates.h"

#include "SEImplementation/Plugin/MeasurementFrameCoordinates/MeasurementFrameCoordinates.h"

#include "SEImplementation/Plugin/Jacobian/Jacobian.h"
#include "SEImplementation/Plugin/Jacobian/JacobianTask.h"

namespace SourceXtractor {

void JacobianGroupTask::computeProperties(SourceGroupInterface &group) const {
  std::cout << "G**\n";
  auto measurement_frame_coordinates = group.begin()->getProperty<MeasurementFrameCoordinates>(m_instance).getCoordinateSystem();
  std::cout << "1**\n";
  auto& detection_group_stamp = group.getProperty<DetectionFrameGroupStamp>();
  std::cout << "2**\n";
  auto detection_frame_coordinates = group.begin()->getProperty<DetectionFrameCoordinates>().getCoordinateSystem();
  std::cout << "##\n";

  double x = detection_group_stamp.getTopLeft().m_x + detection_group_stamp.getStamp().getWidth() / 2.0;
  double y = detection_group_stamp.getTopLeft().m_y + detection_group_stamp.getStamp().getHeight() / 2.0;

  auto frame_origin = measurement_frame_coordinates->worldToImage(detection_frame_coordinates->imageToWorld(ImageCoordinate(x, y)));
  auto frame_dx = measurement_frame_coordinates->worldToImage(
    detection_frame_coordinates->imageToWorld(ImageCoordinate(x + 1.0, y)));
  auto frame_dy = measurement_frame_coordinates->worldToImage(
    detection_frame_coordinates->imageToWorld(ImageCoordinate(x, y + 1.0)));

  group.setIndexedProperty<JacobianGroup>(m_instance,
                                          frame_dx.m_x - frame_origin.m_x, frame_dx.m_y - frame_origin.m_y,
                                          frame_dy.m_x - frame_origin.m_x, frame_dy.m_y - frame_origin.m_y);
}

void JacobianSourceTask::computeProperties(SourceInterface &source) const {
  std::cout << "**\n";
  auto measurement_frame_coordinates = source.getProperty<MeasurementFrameCoordinates>(m_instance).getCoordinateSystem();
  std::cout << "1**\n";
  auto& detection_boundaries = source.getProperty<PixelBoundaries>();
  std::cout << "2**\n";
  auto detection_frame_coordinates = source.getProperty<DetectionFrameCoordinates>().getCoordinateSystem();
  std::cout << "##\n";

  double x = detection_boundaries.getMin().m_x + detection_boundaries.getWidth() / 2.0;
  double y = detection_boundaries.getMin().m_y + detection_boundaries.getHeight() / 2.0;

  auto frame_origin = measurement_frame_coordinates->worldToImage(detection_frame_coordinates->imageToWorld(ImageCoordinate(x, y)));
  auto frame_dx = measurement_frame_coordinates->worldToImage(
    detection_frame_coordinates->imageToWorld(ImageCoordinate(x + 1.0, y)));
  auto frame_dy = measurement_frame_coordinates->worldToImage(
    detection_frame_coordinates->imageToWorld(ImageCoordinate(x, y + 1.0)));

  source.setIndexedProperty<JacobianSource>(m_instance,
                                            frame_dx.m_x - frame_origin.m_x, frame_dx.m_y - frame_origin.m_y,
                                            frame_dy.m_x - frame_origin.m_x, frame_dy.m_y - frame_origin.m_y);
}

} // end SourceXtractor
