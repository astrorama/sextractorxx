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


#ifndef _SEIMPLEMENTATION_PLUGIN_ASSOCMODE_ASSOCMODECONFIG_H_
#define _SEIMPLEMENTATION_PLUGIN_ASSOCMODE_ASSOCMODECONFIG_H_

#include <boost/algorithm/string/split.hpp>

#include "Configuration/Configuration.h"
#include "Table/TableReader.h"

#include "SEFramework/CoordinateSystem/CoordinateSystem.h"

namespace SourceXtractor {

class AssocModeConfig : public Euclid::Configuration::Configuration {

public:

  enum class AssocMode {
    UNKNOWN,
    FIRST,
    NEAREST,
    MIN,
    MAX,
    MEAN,
    MAG_MEAN,
    SUM,
    MAG_SUM
  };

  enum class AssocFilter {
    ALL,
    MATCHED,
    UNMATCHED
  };

  enum class AssocCoordType {
    PIXEL,
    WORLD
  };

  struct CatalogEntry {
    ImageCoordinate coord;
    WorldCoordinate world_coord;
    double weight;
    std::vector<double> assoc_columns;

//    double source_radius_pixels;
    double source_pixel_width;
    double source_pixel_height;

    unsigned int group_id;
  };

  explicit AssocModeConfig(long manager_id);
  virtual ~AssocModeConfig() = default;

  std::map<std::string, OptionDescriptionList> getProgramOptions() override;
  void initialize(const UserValues& args) override;

  AssocMode getAssocMode() const {
    return m_assoc_mode;
  }

  double getAssocRadius() const {
    return m_assoc_radius;
  }

  const std::vector<std::vector<CatalogEntry>>& getCatalogs() const {
    return m_catalogs;
  }

  const std::vector<int>& getColumnsIdx() const {
    return m_columns_idx;
  }

  std::vector<std::string> getColumnsNames() const {
    return m_custom_column_names;
  }

private:
  void readCommonConfig(const UserValues& args);
  void readConfigFromParams(const UserValues& args);
  void readConfigFromFile(const std::string& filename);
  std::map<std::string, unsigned int> parseConfigFile(const std::string& filename);

  void checkConfig();
  void printConfig();
  void readCatalogs(const std::string& filename, const std::vector<int>& columns, AssocCoordType assoc_coord_type);
  AssocCoordType getCoordinateType(const UserValues& args) const;

  std::vector<CatalogEntry> readTable(const Euclid::Table::Table& table, const std::vector<int>& columns,
      const std::vector<int>& copy_columns, bool use_world, std::shared_ptr<CoordinateSystem> coordinate_system=nullptr);

  AssocMode m_assoc_mode;
  double m_assoc_radius;
  double m_default_pixel_size;
  int m_pixel_width_column;
  int m_pixel_height_column;
  int m_group_id_column;

  std::vector<std::vector<CatalogEntry>> m_catalogs;
  std::vector<int> m_columns;
  std::vector<int> m_columns_idx;
  std::vector<std::string> m_custom_column_names;

  std::map<std::string, unsigned int>  m_assoc_columns;
  std::string m_filename;

  AssocCoordType m_assoc_coord_type;
};

} /* namespace SourceXtractor */


#endif /* _SEIMPLEMENTATION_PLUGIN_ASSOCMODE_ASSOCMODECONFIG_H_ */
