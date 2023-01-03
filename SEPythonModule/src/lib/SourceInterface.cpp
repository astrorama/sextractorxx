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

#include "SEPythonModule/SourceInterface.h"
#include "SEFramework/Output/OutputRegistry.h"
#include "SEFramework/Property/DetectionFrame.h"
#include "SEFramework/Source/SourceFactory.h"
#include "SEFramework/Source/SourceGroupWithOnDemandPropertiesFactory.h"
#include "SEFramework/Source/SourceInterface.h"
#include "SEImplementation/Configuration/DetectionFrameConfig.h"
#include "SEImplementation/Plugin/GroupInfo/GroupInfo.h"
#include "SEImplementation/Property/SourceId.h"
#include <ElementsKernel/Logging.h>
#include <NdArray/NdArray.h>
#include <boost/python/numpy.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/variant/static_visitor.hpp>

namespace SourceXPy {

namespace py = boost::python;
namespace np = boost::python::numpy;
using Euclid::NdArray::NdArray;
using SourceXtractor::DetectionFrame;
using SourceXtractor::DetectionFrameConfig;
using SourceXtractor::GroupInfo;
using SourceXtractor::PixelCoordinateList;
using SourceXtractor::PropertyId;
using SourceXtractor::SourceId;

namespace {

auto logger = Elements::Logging::getLogger("SePy");

struct CellToPythonVisitor : public boost::static_visitor<py::object> {

  template <typename T>
  py::object operator()(const std::vector<T>& vector,
                        typename std::enable_if<!std::is_same<T, bool>::value>::type* = nullptr) const {
    auto array = np::zeros(py::make_tuple(vector.size()), np::dtype::get_builtin<T>());
    std::memcpy(array.get_data(), vector.data(), vector.size() * sizeof(T));
    return array;
  }

  template <typename T>
  py::object operator()(const NdArray<T>& nd) const {
    py::list shape;
    boost::for_each(nd.shape(), [&shape](size_t d) { shape.append(d); });
    auto array = np::zeros(py::tuple(shape), np::dtype::get_builtin<T>());
    std::memcpy(array.get_data(), &(*nd.begin()), nd.size() * sizeof(T));
    return array;
  }

  py::object operator()(const std::vector<bool>& vector) const {
    auto   array = np::zeros(py::make_tuple(vector.size()), np::dtype::get_builtin<bool>());
    size_t i     = 0;
    for (auto&& v : vector) {
      array[i] = v;
      ++i;
    }
    return array;
  }

  py::object operator()(const std::string& v) const {
    return py::object(v);
  }

  template <typename From>
  py::object
  operator()(From&& v,
             typename std::enable_if<std::is_pod<typename std::decay<From>::type>::value>::type* = nullptr) const {
    return py::object(v);
  }
};

}  // namespace

std::string DetachedSource::repr() const {
  unsigned int source_id = py::extract<unsigned int>(m_attributes["source_id"]);
  return std::to_string(source_id) + " [DETACHED]";
}

py::list DetachedSource::attributes() const {
  return m_attributes.keys();
}

py::object DetachedSource::attribute(const std::string& key) const {
  return m_attributes[key];
}

py::object AttachedSource::attribute(const std::string& key) const {
  if (!m_source_ptr) {
    PyErr_SetString(PyExc_ReferenceError, "Not owned source is gone");
    throw py::error_already_set();
  }
  // Trigger computation if needed
  try {
    auto       property_type_index = m_context->m_output_registry->getPropertyForColumn(key);
    PropertyId property_id(property_type_index, 0);
    m_source_ptr->getProperty(property_id);
  } catch (const std::out_of_range&) {
    PyErr_SetString(PyExc_AttributeError, key.c_str());
    py::throw_error_already_set();
  }
  // Convert
  const auto& converter = m_context->m_output_registry->getColumnConverter(key);
  return boost::apply_visitor(CellToPythonVisitor(), converter.second(*m_source_ptr));
}

DetachedSource AttachedSource::detach() const {
  using SourceXtractor::Property;
  using SourceXtractor::PropertyId;

  if (!m_source_ptr) {
    PyErr_SetString(PyExc_ReferenceError, "Not owned source is gone");
    py::throw_error_already_set();
  }

  DetachedSource detached;
  const auto&    output_registry = m_context->m_output_registry;
  m_source_ptr->visitProperties([&detached, source_ptr = m_source_ptr,
                                 output_registry](const PropertyId& prop_id, const std::shared_ptr<Property>&) {
    auto columns = output_registry->getColumns(prop_id);
    if (columns.empty()) {
      logger.debug() << "C++ property serialization not found for " << prop_id.getString();
    }
    for (const auto& column : columns) {
      if (detached.m_attributes.contains(column)) {
        continue;
      }
      const auto& converter         = output_registry->getColumnConverter(column);
      detached.m_attributes[column] = boost::apply_visitor(CellToPythonVisitor(), converter.second(*source_ptr));
    }
  });
  return detached;
}

std::string OwnedSource::repr() const {
  auto source_id = m_source_ptr->getProperty<SourceId>().getSourceId();
  return std::to_string(source_id) + " [OWNED]";
}

std::string EntangledSource::repr() const {
  if (!m_source_ptr) {
    PyErr_SetString(PyExc_ReferenceError,
                    "Not owned entangled source is gone. Use its detach method if you need a copy.");
    throw py::error_already_set();
  }
  auto source_id = m_source_ptr->getProperty<SourceId>().getSourceId();
  return std::to_string(source_id) + " [ENTANGLED]";
}

std::string SourceGroup::repr() const {
  auto& group_id = m_group->getProperty<GroupInfo>();
  return std::to_string(group_id.getGroupId());
}

size_t SourceGroup::size() const {
  return m_group->size();
}

py::object SourceGroup::attribute([[maybe_unused]] const std::string& key) const {
  return py::object();
}

SourceGroup::Iterator SourceGroup::iter() const {
  return Iterator{m_group->begin(), m_group->end(), std::make_shared<EntangledSource>(m_context)};
}

std::shared_ptr<EntangledSource> SourceGroup::Iterator::next() {
  if (m_i == m_end) {
    // If anyone is still using it, it will fail on access
    m_holder->m_source_ptr = nullptr;
    PyErr_SetNone(PyExc_StopIteration);
    py::throw_error_already_set();
  }
  m_holder->m_source_ptr = &m_i->getRef();
  ++m_i;
  return m_holder;
}

namespace {

PixelCoordinateList PixelCoordinateFromTuple(const boost::python::tuple& tuple) {
  const np::ndarray                            xs = py::extract<np::ndarray>(tuple[0]);
  const np::ndarray                            ys = py::extract<np::ndarray>(tuple[1]);
  std::vector<SourceXtractor::PixelCoordinate> coordinates(py::len(xs));

  for (std::size_t i = 0; i < coordinates.size(); ++i) {
    coordinates[i].m_x = py::extract<int>(xs[i]);
    coordinates[i].m_y = py::extract<int>(ys[i]);
  }

  return PixelCoordinateList{std::move(coordinates)};
}

}  // namespace

std::shared_ptr<OwnedSource> OwnedSource::create(const std::shared_ptr<Context>& context, int detection_frame_idx,
                                                 int detection_id, const boost::python::tuple& pixels) {
  const auto& detection_frames =
      context->m_config_manager->getConfiguration<DetectionFrameConfig>().getDetectionFrames();
  const auto& detection_frame = detection_frames.at(detection_frame_idx);

  auto source_ptr = context->m_source_factory->createSource();
  source_ptr->setProperty<DetectionFrame>(detection_frame);
  source_ptr->setProperty<SourceId>(detection_id);
  source_ptr->setProperty<PixelCoordinateList>(PixelCoordinateFromTuple(pixels));

  return std::make_shared<OwnedSource>(context, std::move(source_ptr));
}

std::shared_ptr<SourceGroup> SourceGroup::create(const std::shared_ptr<Context>& context,
                                                 boost::python::list&            sources) {
  auto group_ptr = context->m_group_factory->createSourceGroup();

  auto n_sources = len(sources);
  for (ssize_t i = 0; i < n_sources; ++i) {
    const auto& source = py::extract<std::shared_ptr<OwnedSource>>(sources[i]);
    // TODO: Do not clone?
    group_ptr->addSource(source()->m_owned_source->clone());
  }

  sources.attr("clear")();
  return std::make_shared<SourceGroup>(SourceGroup{std::move(group_ptr), context});
}

}  // namespace SourceXPy
