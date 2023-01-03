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
 * @file PythonConfig.h
 * @author Nikolaos Apostolakos <nikoapos@gmail.com>
 */

#ifndef _SEIMPLEMENTATION_CONFIGURATION_PYTHONCONFIG_H
#define _SEIMPLEMENTATION_CONFIGURATION_PYTHONCONFIG_H

#include <Configuration/Configuration.h>
#include <SEImplementation/PythonConfig/PythonInterpreter.h>

namespace SourceXtractor {

class PythonConfig : public Euclid::Configuration::Configuration {

public:

  explicit PythonConfig(long manager_id);

  std::map<std::string, Configuration::OptionDescriptionList> getProgramOptions() override;

  void preInitialize(const UserValues& args) override;

  void initialize(const UserValues& args) override;

  PythonInterpreter& getInterpreter() const;

private:
  bool                  m_capture_output = true;
  boost::python::object m_measurement_config;
};

}

#endif // _SEIMPLEMENTATION_CONFIGURATION_PYTHONCONFIG_H

