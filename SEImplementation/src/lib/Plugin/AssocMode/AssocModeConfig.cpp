/** Copyright © 2021 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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

#include <map>
#include <boost/algorithm/string.hpp>
#include <fstream>

#include <CCfits/CCfits>

#include "ElementsKernel/Logging.h"

#include "Table/AsciiReader.h"
#include "Table/FitsReader.h"
#include "Table/CastVisitor.h"

#include "SEImplementation/Configuration/DetectionImageConfig.h"
#include "SEImplementation/Configuration/PartitionStepConfig.h"
#include "SEImplementation/Configuration/MultiThresholdPartitionConfig.h"

#include "SEImplementation/Plugin/AssocMode/AssocModePartitionStep.h"

#include "SEImplementation/Plugin/AssocMode/AssocModeConfig.h"

using namespace Euclid::Configuration;
namespace po = boost::program_options;

namespace SourceXtractor {

static Elements::Logging logger = Elements::Logging::getLogger("AssocModeConfig");

static const std::string ASSOC_CATALOG { "assoc-catalog" };
static const std::string ASSOC_MODE { "assoc-mode" };
static const std::string ASSOC_RADIUS { "assoc-radius" };
static const std::string ASSOC_FILTER { "assoc-filter" };
static const std::string ASSOC_COPY { "assoc-copy" };
static const std::string ASSOC_COLUMNS { "assoc-columns" };
static const std::string ASSOC_COORD_TYPE { "assoc-coord-type" };
static const std::string ASSOC_SOURCE_SIZES { "assoc-source-sizes" };
static const std::string ASSOC_SOURCE_WIDTHS { "assoc-source-widths" };
static const std::string ASSOC_SOURCE_HEIGHTS { "assoc-source-heights" };
static const std::string ASSOC_DEFAULT_PIXEL_SIZE { "assoc-default-pixel-size" };
static const std::string ASSOC_GROUP_ID { "assoc-group-id" };
static const std::string ASSOC_CONFIG { "assoc-config" };
static const std::string ASSOC_TEST { "assoc-test" };

namespace {

const std::map<std::string, AssocModeConfig::AssocMode> assoc_mode_table {
  std::make_pair("", AssocModeConfig::AssocMode::UNKNOWN),
  std::make_pair("FIRST", AssocModeConfig::AssocMode::FIRST),
  std::make_pair("NEAREST", AssocModeConfig::AssocMode::NEAREST),
  std::make_pair("MEAN", AssocModeConfig::AssocMode::MEAN),
  std::make_pair("MAG_MEAN", AssocModeConfig::AssocMode::MAG_MEAN),
  std::make_pair("SUM", AssocModeConfig::AssocMode::SUM),
  std::make_pair("MAG_SUM", AssocModeConfig::AssocMode::MAG_SUM),
  std::make_pair("MIN", AssocModeConfig::AssocMode::MIN),
  std::make_pair("MAX", AssocModeConfig::AssocMode::MAX)
};

const std::map<std::string, AssocModeConfig::AssocFilter> assoc_filter_table {
  std::make_pair("ALL", AssocModeConfig::AssocFilter::ALL),
  std::make_pair("MATCHED", AssocModeConfig::AssocFilter::MATCHED),
  std::make_pair("UNMATCHED", AssocModeConfig::AssocFilter::UNMATCHED)
};

const std::map<std::string, AssocModeConfig::AssocCoordType> assoc_coord_type_table {
  std::make_pair("PIXEL", AssocModeConfig::AssocCoordType::PIXEL),
  std::make_pair("WORLD", AssocModeConfig::AssocCoordType::WORLD)
};

std::vector<int> parseColumnList(const std::string& arg) {
  if (arg.size() > 0) {
    try {
      std::vector<std::string> parts;
      boost::split(parts, arg, boost::is_any_of(","));

      std::vector<int> column_list;
      for (auto& part : parts) {
        // the input is a 1-based index, the internal index is 0-based
        column_list.emplace_back(boost::lexical_cast<int>(part)-1);
      }
      return column_list;
    } catch(...) {
      throw Elements::Exception() << "Can't parse column list to int: " << arg;
    }
  } else {
    return {};
  }
}

}

AssocModeConfig::AssocModeConfig(long manager_id) : Configuration(manager_id), m_assoc_mode(AssocMode::UNKNOWN),
    m_assoc_radius(0.), m_default_pixel_size(10), m_pixel_width_column(-1), m_pixel_height_column(-1),
    m_group_id_column(-1), m_assoc_coord_type(AssocCoordType::PIXEL) {
  declareDependency<DetectionImageConfig>();
  declareDependency<PartitionStepConfig>();

  // this is used to enforce the order the PartitionSteps are added and performed
  ConfigManager::getInstance(manager_id).registerDependency<AssocModeConfig, MultiThresholdPartitionConfig>();
}

std::map<std::string, Configuration::OptionDescriptionList> AssocModeConfig::getProgramOptions() {
  return { {"Assoc config", {
      {ASSOC_CATALOG.c_str(), po::value<std::string>(),
          "Assoc catalog file"},
      {ASSOC_COLUMNS.c_str(), po::value<std::string>()->default_value("1,2"),
          "Assoc columns to specify x/ra,y/dec[,weight] (the index of the first column is 1)"},
      {ASSOC_MODE.c_str(), po::value<std::string>()->default_value("NEAREST"),
          "Assoc mode [FIRST, NEAREST, MEAN, MAG_MEAN, SUM, MAG_SUM, MIN, MAX]"},
      {ASSOC_RADIUS.c_str(), po::value<double>()->default_value(2.0),
          "Assoc radius (Always in pixels of the detection image)"},
      {ASSOC_FILTER.c_str(), po::value<std::string>()->default_value("ALL"),
          "Assoc catalog filter setting: ALL, MATCHED, UNMATCHED"},
      {ASSOC_COPY.c_str(), po::value<std::string>()->default_value(""),
          "List of columns indices in the assoc catalog to copy on match (the index of the first column is 1). "},
      {ASSOC_COORD_TYPE.c_str(), po::value<std::string>()->default_value("PIXEL"),
          "Assoc coordinates type: PIXEL, WORLD"},
      {ASSOC_SOURCE_SIZES.c_str(), po::value<int>()->default_value(-1),
          "Column containing the source sizes (in reference frame pixels)"},
      {ASSOC_SOURCE_WIDTHS.c_str(), po::value<int>()->default_value(-1),
          "Column containing the source widths (in reference frame pixels)"},
      {ASSOC_SOURCE_HEIGHTS.c_str(), po::value<int>()->default_value(-1),
          "Column containing the source heights (in reference frame pixels)"},
      {ASSOC_DEFAULT_PIXEL_SIZE.c_str(), po::value<double>()->default_value(5.0),
          "Default source size (in reference frame pixels)"},
      {ASSOC_GROUP_ID.c_str(), po::value<int>()->default_value(-1),
          "Column containing the group id"},
      {ASSOC_CONFIG.c_str(), po::value<std::string>(),
          "Text file containing the assoc columns configuration"},
      {ASSOC_TEST.c_str(), po::bool_switch(),
          "Prints the assoc configuration and quits"},
  }}};
}

void AssocModeConfig::initialize(const UserValues& args) {
  // read configuration from command line arguments
  readCommonConfig(args);

  // read columns from columns config file
  if (args.find(ASSOC_CONFIG) != args.end()) {
    auto assoc_config_filename = args.at(ASSOC_CONFIG).as<std::string>();
    readConfigFromFile(assoc_config_filename);
  } else {
    readConfigFromParams(args);
  }

  // sanity check that the configuration is coherent

  // read the catalogs
  if (m_filename != "") {
    checkConfig();
    readCatalogs(m_filename, m_columns, m_assoc_coord_type);

    if (args.at(ASSOC_TEST).as<bool>()) {
      printConfig();
      throw Elements::Exception() << "Exiting by user request";
    }
  }
}

void AssocModeConfig::readCommonConfig(const UserValues& args) {
  auto filter = boost::to_upper_copy(args.at(ASSOC_FILTER).as<std::string>());
  if (assoc_filter_table.find(filter) != assoc_filter_table.end()) {
    auto assoc_filter = assoc_filter_table.at(filter);
    if (assoc_filter == AssocFilter::MATCHED) {
      getDependency<PartitionStepConfig>().addPartitionStepCreator(
              [](std::shared_ptr<SourceFactory> ) {
                return std::make_shared<AssocModePartitionStep>(true);
              }
      );
    } else if (assoc_filter == AssocFilter::UNMATCHED) {
      getDependency<PartitionStepConfig>().addPartitionStepCreator(
              [](std::shared_ptr<SourceFactory> ) {
                return std::make_shared<AssocModePartitionStep>(false);
              }
      );
    }
  } else {
    throw Elements::Exception() << "Invalid assoc filter: " << filter;
  }

  if (args.find(ASSOC_MODE) != args.end()) {
    auto assoc_mode = boost::to_upper_copy(args.at(ASSOC_MODE).as<std::string>());
    if (assoc_mode_table.find(assoc_mode) != assoc_mode_table.end()) {
      m_assoc_mode = assoc_mode_table.at(assoc_mode);
    } else {
      throw Elements::Exception() << "Invalid association mode: " << assoc_mode;
    }
  }

  m_assoc_radius = args.at(ASSOC_RADIUS).as<double>();
  m_default_pixel_size = args.at(ASSOC_DEFAULT_PIXEL_SIZE).as<double>();

  if (args.find(ASSOC_CATALOG) != args.end()) {
    m_filename = args.at(ASSOC_CATALOG).as<std::string>();
  }
}

void AssocModeConfig::readConfigFromParams(const UserValues& args) {
  m_columns =  parseColumnList(args.at(ASSOC_COLUMNS).as<std::string>());
  m_columns_idx = parseColumnList(args.at(ASSOC_COPY).as<std::string>());

  m_pixel_width_column = args.at(ASSOC_SOURCE_SIZES).as<int>() - 1; // config uses 1 as first column
  m_pixel_height_column = args.at(ASSOC_SOURCE_SIZES).as<int>() - 1; // config uses 1 as first column

  if (args.find(ASSOC_SOURCE_WIDTHS) != args.end()) {
    m_pixel_width_column = args.at(ASSOC_SOURCE_WIDTHS).as<int>() - 1; // config uses 1 as first column
  }
  if (args.find(ASSOC_SOURCE_HEIGHTS) != args.end()) {
    m_pixel_height_column = args.at(ASSOC_SOURCE_HEIGHTS).as<int>() - 1; // config uses 1 as first column
  }

  m_group_id_column = args.at(ASSOC_GROUP_ID).as<int>() - 1; // config uses 1 as first column

  m_assoc_coord_type = getCoordinateType(args);
}

void AssocModeConfig::readConfigFromFile(const std::string& filename) {
  m_assoc_columns = parseConfigFile(filename);

  if (m_assoc_columns.find("x") != m_assoc_columns.end() && m_assoc_columns.find("y") != m_assoc_columns.end()) {
    m_assoc_coord_type = AssocCoordType::PIXEL;

    m_columns.push_back(m_assoc_columns.at("x"));
    m_assoc_columns.erase("x");
    m_columns.push_back(m_assoc_columns.at("y"));
    m_assoc_columns.erase("y");

    if (m_assoc_columns.find("ra") != m_assoc_columns.end() ||
          m_assoc_columns.find("dec") != m_assoc_columns.end()) {
      throw Elements::Exception() << "Use either X/Y or RA/DEC coordinates in assoc config file but not both";
    }
  } else if (m_assoc_columns.find("ra") != m_assoc_columns.end() &&
      m_assoc_columns.find("dec") != m_assoc_columns.end()) {

    m_assoc_coord_type = AssocCoordType::WORLD;

    m_columns.push_back(m_assoc_columns.at("ra"));
    m_assoc_columns.erase("ra");
    m_columns.push_back(m_assoc_columns.at("dec"));
    m_assoc_columns.erase("dec");

    if (m_assoc_columns.find("x") != m_assoc_columns.end() ||
          m_assoc_columns.find("y") != m_assoc_columns.end()) {
      throw Elements::Exception() << "Use either X/Y or RA/DEC coordinates in assoc config file but not both";
    }
  } else {
    throw Elements::Exception() << "Missing X/Y or RA/DEC coordinates in assoc config file";
  }

  if (m_assoc_columns.find("weight") != m_assoc_columns.end()) {
    m_columns.push_back(m_assoc_columns.at("weight"));
    m_assoc_columns.erase("weight");
  }

  if (m_assoc_columns.find("pixel_size") != m_assoc_columns.end()) {
    m_pixel_width_column = m_assoc_columns.at("pixel_size");
    m_pixel_height_column = m_assoc_columns.at("pixel_size");
    m_assoc_columns.erase("pixel_size");
  }

  if (m_assoc_columns.find("pixel_width") != m_assoc_columns.end()) {
    m_pixel_width_column = m_assoc_columns.at("pixel_width");
    m_assoc_columns.erase("pixel_width");
  }

  if (m_assoc_columns.find("pixel_height") != m_assoc_columns.end()) {
    m_pixel_width_column = m_assoc_columns.at("pixel_height");
    m_assoc_columns.erase("pixel_height");
  }

  if (m_assoc_columns.find("group_id") != m_assoc_columns.end()) {
    m_group_id_column = m_assoc_columns.at("group_id");
    m_assoc_columns.erase("group_id");
  }

  for (auto& column_info : m_assoc_columns) {
    m_custom_column_names.push_back(column_info.first);
    m_columns_idx.push_back(column_info.second);
  }
}

void AssocModeConfig::checkConfig() {
  if (m_columns.size() < 2) {
    throw Elements::Exception() << "At least 2 columns must be specified for x,y coordinates in the assoc catalog";
  }
  if (m_columns.size() > 3) {
    throw Elements::Exception() << "Maximum 3 columns for x, y and weight must be specified in the assoc catalog";
  }

  if (m_assoc_coord_type == AssocCoordType::PIXEL && getDependency<DetectionImageConfig>().getExtensionsNb() > 1) {
    logger.warn() <<
        "Using Assoc catalog matching in pixel coordinates with multiple detection images";
  }

  if (m_assoc_coord_type == AssocCoordType::PIXEL && getDependency<DetectionImageConfig>().getExtensionsNb() < 1) {
    throw Elements::Exception() << "Using Assoc catalog matching in pixel coordinates without a detection images";
  }

}

AssocModeConfig::AssocCoordType AssocModeConfig::getCoordinateType(const UserValues& args) const {
  AssocCoordType assoc_coord_type = AssocCoordType::PIXEL;
  if (args.find(ASSOC_COORD_TYPE) != args.end()) {
    auto assoc_coord_type_str = boost::to_upper_copy(args.at(ASSOC_COORD_TYPE).as<std::string>());
    if (assoc_coord_type_table.find(assoc_coord_type_str) != assoc_coord_type_table.end()) {
      assoc_coord_type = assoc_coord_type_table.at(assoc_coord_type_str);
    } else {
      throw Elements::Exception() << "Invalid association coordinate type: " << assoc_coord_type_str;
    }
  }
  return assoc_coord_type;
}

void AssocModeConfig::readCatalogs(const std::string& filename,
    const std::vector<int>& columns, AssocCoordType assoc_coord_type) {
  try {
    std::shared_ptr<Euclid::Table::TableReader> reader;
    try {
      reader = std::make_shared<Euclid::Table::FitsReader>(filename);
    } catch (...) {
      // If FITS not successful try reading as ascii
      reader = std::make_shared<Euclid::Table::AsciiReader>(filename);
    }
    auto table = reader->read();

    size_t exts_nb = getDependency<DetectionImageConfig>().getExtensionsNb();
    if (exts_nb == 0) {
      // No detection image
      m_catalogs.emplace_back(readTable(table, columns, m_columns_idx, true));
    } else {
      for (size_t i = 0; i < exts_nb; i++) {
        auto coordinate_system = getDependency<DetectionImageConfig>().getCoordinateSystem(i);
        if (assoc_coord_type == AssocCoordType::WORLD) {
          m_catalogs.emplace_back(readTable(table, columns, m_columns_idx, true, coordinate_system));
        } else {
          m_catalogs.emplace_back(readTable(table, columns, m_columns_idx, false, coordinate_system));
        }
      }
    }
  } catch (const std::exception& e) {
    throw Elements::Exception() << "Can't either open or read assoc catalog: " << filename << " (" << e.what() << ")";
  } catch(...) {
    throw Elements::Exception() << "Can't either open or read assoc catalog: " << filename;
  }
}

std::vector<AssocModeConfig::CatalogEntry> AssocModeConfig::readTable(
    const Euclid::Table::Table& table, const std::vector<int>& columns,
    const std::vector<int>& copy_columns, bool use_world, std::shared_ptr<CoordinateSystem> coordinate_system) {
  using Euclid::Table::CastVisitor;

  std::vector<CatalogEntry> catalog;
  for (auto& row : table) {

    ImageCoordinate coord;
    WorldCoordinate world_coord;
    if (use_world) {
      world_coord = WorldCoordinate {
          boost::apply_visitor(CastVisitor<double>{}, row[columns.at(0)]),
          boost::apply_visitor(CastVisitor<double>{}, row[columns.at(1)]),
      };
      if (coordinate_system != nullptr) {
        coord = coordinate_system->worldToImage(world_coord);
      }
    } else {
      coord = ImageCoordinate {
          // our internal pixel coordinates are zero-based
          boost::apply_visitor(CastVisitor<double>{}, row[columns.at(0)]) - 1.0,
          boost::apply_visitor(CastVisitor<double>{}, row[columns.at(1)]) - 1.0,
      };
      if (coordinate_system != nullptr) {
        world_coord = coordinate_system->imageToWorld(coord);
      }
    }
    catalog.emplace_back(CatalogEntry { coord, world_coord, 1.0, {}, 1.0, 1.0, 0 });
    if (columns.size() == 3 && columns.at(2) >= 0) {
      catalog.back().weight = boost::apply_visitor(CastVisitor<double>{}, row[columns.at(2)]);
    }
    for (auto column : copy_columns) {
      if (column >= static_cast<int>(row.size())) {
        throw Elements::Exception() << "Column index " << column << " is out of bounds";
      }
      if (row[column].type() == typeid(int)) {
        catalog.back().assoc_columns.emplace_back(boost::get<int>(row[column]));
      } else if (row[column].type() == typeid(double)) {
        catalog.back().assoc_columns.emplace_back(boost::get<double>(row[column]));
      } else if (row[column].type() == typeid(int64_t)) {
        catalog.back().assoc_columns.emplace_back(boost::get<int64_t>(row[column]));
      } else if (row[column].type() == typeid(SeFloat)) {
        catalog.back().assoc_columns.emplace_back(boost::get<SeFloat>(row[column]));
      } else {
        throw Elements::Exception() << "Wrong type in assoc column (must be a numeric type)";
      }
    }

    if (m_group_id_column >= 0) {
      catalog.back().group_id = boost::apply_visitor(CastVisitor<int64_t>{}, row[m_group_id_column]);
    }

    if (m_pixel_width_column >= 0 && m_pixel_height_column >= 0) {
      //catalog.back().source_radius_pixels = boost::apply_visitor(CastVisitor<double>{}, row[m_pixel_size_column]);
      catalog.back().source_pixel_width = boost::apply_visitor(CastVisitor<double>{}, row[m_pixel_width_column]);
      catalog.back().source_pixel_height = boost::apply_visitor(CastVisitor<double>{}, row[m_pixel_height_column]);
    } else {
      catalog.back().source_pixel_width = m_default_pixel_size;
      catalog.back().source_pixel_height = m_default_pixel_size;
    }
  }
  return catalog;
}

std::map<std::string, unsigned int>  AssocModeConfig::parseConfigFile(const std::string& filename) {
    std::map<std::string, unsigned int> columns;

    const std::vector<std::string> reserved_names {
      "x", "y", "ra", "dec", "weight", "group_id", "pixel_size", "pixel_width", "pixel_height"
    };

    std::ifstream config_file(filename);
    if (!config_file.is_open()) {
        throw Elements::Exception() << "Can't either open or read assoc config file: " << filename;
    }

    std::string line;
    int current_column_nb = 1; // column indices start at 1
    while (std::getline(config_file, line)) {
      boost::trim(line);
      // Skip lines starting with '#'
      if (line.empty() || line[0] == '#') {
          continue;
      }

      std::string name;
      int number;
      // Find the position of the '=' character
      auto equal_sign_pos = line.find('=');
      if (equal_sign_pos != std::string::npos) {
          name = line.substr(0, equal_sign_pos);
          std::string number_string = line.substr(equal_sign_pos + 1);
          std::istringstream iss(number_string);
          iss >> number;
          number--;
          current_column_nb = number + 1;
      } else {
          name = line;
          number = current_column_nb++;
      }
      boost::trim(name);

      if (std::find(reserved_names.begin(), reserved_names.end(), boost::to_lower_copy(name)) != reserved_names.end()) {
        boost::to_lower(name);
      }


      // Store the parsed information into the vector
      columns[name] = number;
    }

    config_file.close();
    return columns;
}

void AssocModeConfig::printConfig() {
  std::cout << "Assoc catalog configuration" << std::endl;

  auto& catalog = m_catalogs.at(0);

  if (m_assoc_coord_type == AssocModeConfig::AssocCoordType::PIXEL) {
    std::cout << "X" << "\t";
    std::cout << "Y" << "\t";
  } else {
    std::cout << "RA" << "\t";
    std::cout << "DEC" << "\t";
  }
  if (m_columns.size() >= 3) {
    std::cout << "WEIGHT" << "\t";
  }
  if (m_pixel_width_column >= 0) {
    std::cout << "PIXEL_WIDTH" << "\t";
  }
  if (m_pixel_height_column >= 0) {
    std::cout << "PIXEL_HEIGHT" << "\t";
  }
  if (m_group_id_column >= 0) {
    std::cout << "GROUP_ID" << "\t";
  }

  for (const auto& name : m_custom_column_names) {
    std::cout << name << "\t";
  }
  std::cout << std::endl;

  int lines = 0;
  for (const auto& entry : catalog) {
    if (m_assoc_coord_type == AssocModeConfig::AssocCoordType::PIXEL) {
      std::cout << entry.coord.m_x << "\t";
      std::cout << entry.coord.m_y << "\t";
    } else {
      std::cout << entry.world_coord.m_alpha << "\t";
      std::cout << entry.world_coord.m_delta << "\t";
    }
    if (m_columns.size() >= 3) {
      std::cout << entry.weight << "\t";
    }
    if (m_pixel_width_column >= 0) {
      std::cout << entry.source_pixel_width << "\t";
    }
    if (m_pixel_height_column >= 0) {
      std::cout << entry.source_pixel_height << "\t";
    }
    if (m_group_id_column >= 0) {
      std::cout << entry.group_id << "\t";
    }
    for (auto& column : entry.assoc_columns) {
      std::cout << column << "\t";
    }
    std::cout << std::endl;

    if (++lines >= 10) {
      break;
    }
  }
}


}
