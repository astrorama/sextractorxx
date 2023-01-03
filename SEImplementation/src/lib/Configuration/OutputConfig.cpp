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
/**
 * @file src/lib/Configuration/OutputConfig.cpp
 * @date 06/23/16
 * @author mschefer
 */

#include <sstream>

#include <boost/algorithm/string.hpp>

#include "ElementsKernel/Exception.h"

#include "SEImplementation/Plugin/PixelCentroid/PixelCentroid.h"
#include "SEImplementation/Plugin/PixelBoundaries/PixelBoundaries.h"

#include "SEImplementation/Configuration/OutputConfig.h"

using namespace Euclid::Configuration;
namespace po = boost::program_options;

namespace SourceXtractor {

static const std::string OUTPUT_FILE {"output-catalog-filename"};
static const std::string OUTPUT_FILE_FORMAT {"output-catalog-format"};
static const std::string OUTPUT_PROPERTIES {"output-properties"};
static const std::string OUTPUT_FLUSH_SIZE {"output-flush-size"};
static const std::string OUTPUT_SORTED {"output-flush-sorted"};

static std::map<std::string, OutputConfig::OutputFileFormat> format_map{
  {"ASCII",     OutputConfig::OutputFileFormat::ASCII},
  {"FITS",      OutputConfig::OutputFileFormat::FITS},
  {"FITS_LDAC", OutputConfig::OutputFileFormat::FITS_LDAC}
};

OutputConfig::OutputConfig(long manager_id) : Configuration(manager_id), m_format(OutputFileFormat::ASCII),
                                              m_flush_size(100), m_unsorted(false) {
}

std::map<std::string, Configuration::OptionDescriptionList> OutputConfig::getProgramOptions() {
  return { {"Output configuration", {
      {OUTPUT_FILE.c_str(), po::value<std::string>()->default_value(""),
          "The file to store the output catalog"},
      {OUTPUT_FILE_FORMAT.c_str(), po::value<std::string>()->default_value("FITS"),
          "The format of the output catalog, one of ASCII, FITS or FITS_LDAC (default: FITS)"},
      {OUTPUT_PROPERTIES.c_str(), po::value<std::string>()->default_value("PixelCentroid"),
          "The output properties to add in the output catalog"},
      {OUTPUT_FLUSH_SIZE.c_str(), po::value<int>()->default_value(100),
         "Write to the catalog after this number of sources have been processed (0 means once at the end)"},
      {OUTPUT_SORTED.c_str(), po::value<bool>()->default_value(true),
         "Delay the output of some sources to ensure a deterministic order"}
  }}};
}

void OutputConfig::preInitialize(const UserValues& args) {
  auto format_name = boost::to_upper_copy(args.at(OUTPUT_FILE_FORMAT).as<std::string>());
  if (format_map.count(format_name) == 0) {
    throw Elements::Exception() << "Unknown output file format: " << format_name;
  }
}

void OutputConfig::initialize(const UserValues& args) {
  m_out_file = args.at(OUTPUT_FILE).as<std::string>();

  std::stringstream properties_str {args.at(OUTPUT_PROPERTIES).as<std::string>()};
  std::string name;
  while (std::getline(properties_str, name, ',')) {
    m_output_properties.emplace_back(name);
  }

  auto format_name = boost::to_upper_copy(args.at(OUTPUT_FILE_FORMAT).as<std::string>());
  m_format = format_map.at(format_name);

  int flush_size = args.at(OUTPUT_FLUSH_SIZE).as<int>();
  m_flush_size = (flush_size >= 0) ? flush_size : 0;

  m_unsorted = !args.at(OUTPUT_SORTED).as<bool>();
}

std::string OutputConfig::getOutputFile() const {
  return m_out_file;
}

OutputConfig::OutputFileFormat OutputConfig::getOutputFileFormat() const {
  return m_format;
}

const std::vector<std::string> OutputConfig::getOutputProperties() const {
  return m_output_properties;
}

size_t OutputConfig::getFlushSize() const {
  return m_flush_size;
}

bool OutputConfig::getOutputUnsorted() const {
  return m_unsorted;
}

} // SEImplementation namespace



