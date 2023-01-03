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
 * @file SEImplementation/Configuration/OutputConfig.h
 * @date 06/23/16
 * @author mschefer
 */

#ifndef _SEIMPLEMENTATION_CONFIGURATION_OUTPUTCONFIG_H
#define _SEIMPLEMENTATION_CONFIGURATION_OUTPUTCONFIG_H

#include "Configuration/Configuration.h"
#include "SEFramework/Output/OutputRegistry.h"

namespace SourceXtractor {

/**
 * @class OutputConfig
 * @brief
 *
 */
class OutputConfig : public Euclid::Configuration::Configuration {

public:
  
  enum class OutputFileFormat {
    ASCII, FITS, FITS_LDAC
  };
  
  /// Destructor
  virtual ~OutputConfig() = default;

  /// Constructor
  explicit OutputConfig(long manager_id);

  std::map<std::string, Configuration::OptionDescriptionList> getProgramOptions() override;
  
  void preInitialize(const UserValues& args) override;

  void initialize(const UserValues& args) override;
  
  std::string getOutputFile() const;
  
  OutputFileFormat getOutputFileFormat() const;
  
  const std::vector<std::string> getOutputProperties() const;

  size_t getFlushSize() const;

  bool getOutputUnsorted() const;

private:
 
  std::string m_out_file;
  OutputFileFormat m_format;
  std::vector<std::string> m_output_properties;
  size_t m_flush_size;
  bool m_unsorted;

}; /* End of OutputConfig class */

} /* namespace SourceXtractor */


#endif
