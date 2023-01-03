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
 * @file PythonModule.cpp
 * @author Nikolaos Apostolakos <nikoapos@gmail.com>
 */

#include <boost/python/class.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/module.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>

#include <SEFramework/Source/SourceFlags.h>
#include <SEImplementation/PythonConfig/PyMeasurementImage.h>
#include <SEImplementation/PythonConfig/PyAperture.h>
#include <SEImplementation/PythonConfig/PythonModule.h>
#include <SEImplementation/PythonConfig/PyOutputWrapper.h>
#include <SEImplementation/PythonConfig/PyFitsFile.h>

namespace bp = boost::python;

namespace SourceXtractor {

BOOST_PYTHON_MODULE(_SEPythonConfig) {

  bp::class_<PyOutputWrapper, boost::noncopyable>("OutputWrapper",
    "A file-like object used to wrap stdout and stderr", bp::no_init)
    .def_readonly("closed", &PyOutputWrapper::closed)
    .def("close", &PyOutputWrapper::close)
    .def("fileno", &PyOutputWrapper::fileno)
    .def("flush", &PyOutputWrapper::flush)
    .def("isatty", &PyOutputWrapper::isatty)
    .def("readable", &PyOutputWrapper::readable)
    .def("read", &PyOutputWrapper::read)
    .def("readline", &PyOutputWrapper::readline)
    .def("readlines", &PyOutputWrapper::readlines)
    .def("seek", &PyOutputWrapper::seek)
    .def("seekable", &PyOutputWrapper::seekable)
    .def("tell", &PyOutputWrapper::tell)
    .def("truncate", &PyOutputWrapper::truncate)
    .def("writable", &PyOutputWrapper::writable)
    .def("write", &PyOutputWrapper::write)
    .def("writelines", &PyOutputWrapper::writelines);

  bp::class_<PyMeasurementImage>("MeasurementImage",
    "C++ part of the MeasurementImage", bp::init<std::string, std::string, std::string>())
      .def_readonly("id", &PyMeasurementImage::id)
      .def_readonly("file", &PyMeasurementImage::file)
      .def_readwrite("gain", &PyMeasurementImage::gain)
      .def_readwrite("saturation", &PyMeasurementImage::saturation)
      .def_readwrite("flux_scale", &PyMeasurementImage::flux_scale)
      .def_readonly("psf_file", &PyMeasurementImage::psf_file)
      .def_readonly("weight_file", &PyMeasurementImage::weight_file)
      .def_readwrite("weight_type", &PyMeasurementImage::weight_type)
      .def_readwrite("weight_absolute", &PyMeasurementImage::weight_absolute)
      .def_readwrite("weight_scaling", &PyMeasurementImage::weight_scaling)
      .def_readwrite("has_weight_threshold", &PyMeasurementImage::has_weight_threshold)
      .def_readwrite("weight_threshold", &PyMeasurementImage::weight_threshold)
      .def_readwrite("is_background_constant", &PyMeasurementImage::is_background_constant)
      .def_readwrite("constant_background_value", &PyMeasurementImage::constant_background_value)
      .def_readwrite("image_hdu", &PyMeasurementImage::image_hdu)
      .def_readwrite("psf_hdu", &PyMeasurementImage::psf_hdu)
      .def_readwrite("weight_hdu", &PyMeasurementImage::weight_hdu)
      .def_readwrite("is_data_cube", &PyMeasurementImage::is_data_cube)
      .def_readwrite("image_layer", &PyMeasurementImage::image_layer)
      .def_readwrite("weight_layer", &PyMeasurementImage::weight_layer)
      ;

  bp::class_<PyId>("Id", bp::init<>())
    .def_readonly("id", &PyId::id);

  bp::class_<PyAperture, bp::bases<PyId>>("Aperture",
    "Set of aperture photometries", bp::init<bp::list>())
    .def_readonly("apertures", &PyAperture::apertures, "List of aperture diameters, in pixels")
    .def("__str__", &PyAperture::toString)
    .def("__repr__", &PyAperture::toString);

  bp::class_<CoordinateSystem, boost::noncopyable>("CoordinateSystem",
    "Implements transformation of coordinates between image and world coordinates", bp::no_init)
      .def("image_to_world", &CoordinateSystem::imageToWorld)
      .def("world_to_image", &CoordinateSystem::worldToImage);
  bp::register_ptr_to_python<std::shared_ptr<CoordinateSystem>>();

  bp::class_<WorldCoordinate>("WorldCoordinate", "World coordinates")
      .def(bp::init<double, double>())
      .def_readwrite("alpha", &WorldCoordinate::m_alpha)
      .def_readwrite("delta", &WorldCoordinate::m_delta);

  bp::class_<ImageCoordinate>("ImageCoordinate", "Image coordinates, in pixels")
      .def(bp::init<double, double>())
      .def_readwrite("x", &ImageCoordinate::m_x)
      .def_readwrite("y", &ImageCoordinate::m_y);

  bp::enum_<Flags>("Flags", "Source flags")
      .value("NONE", Flags::NONE)
      .value("BIASED", Flags::BIASED)
      .value("SATURATED", Flags::SATURATED)
      .value("BOUNDARY", Flags::BOUNDARY)
      .value("NEIGHBORS", Flags::NEIGHBORS)
      .value("OUTSIDE", Flags::OUTSIDE)
      .value("PARTIAL_FIT", Flags::PARTIAL_FIT)
      .value("INSUFFICIENT_DATA", Flags::INSUFFICIENT_DATA)
      .value("ERROR", Flags::ERROR);

  bp::class_<std::vector<int> >("_IntVector")
    .def(bp::vector_indexing_suite<std::vector<int> >());

  bp::class_<std::vector<unsigned int> >("_UIntVector")
    .def(bp::vector_indexing_suite<std::vector<unsigned int> >());

  bp::class_<std::map<std::string, std::string>>("_StringStringMap")
    .def(bp::map_indexing_suite<std::map<std::string, std::string>>());

  bp::class_<PyFitsFile>("FitsFile", "A FITS file opened by SourceXtractor++", bp::init<const std::string&>())
    .def_readonly("filename", &PyFitsFile::getFilename)
    .def_readonly("image_hdus", &PyFitsFile::getImageHdus)
    .def("get_headers", &PyFitsFile::getHeaders, "get headers for hdu")
    .def("get_dimensions", &PyFitsFile::getDimensions, "get dimensions for hdu");

}

} // namespace SourceXtractor
