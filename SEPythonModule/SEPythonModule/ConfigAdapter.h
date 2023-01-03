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

#ifndef SOURCEXTRACTORPLUSPLUS_CONFIGADAPTER_H
#define SOURCEXTRACTORPLUSPLUS_CONFIGADAPTER_H

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/python/dict.hpp>
#include <map>

namespace SourceXPy {

class ConfigAdapter {
public:
  using config_map_t = std::map<std::string, boost::program_options::variable_value>;

  explicit ConfigAdapter(boost::program_options::options_description opt_desc);

  void fromPython(const boost::python::dict& config);

  template <typename T>
  void set(const std::string& key, T&& value) {
    m_options[key] = boost::program_options::variable_value(std::forward<T>(value), false);
  }

  const config_map_t& getOptions() const;

private:
  boost::program_options::options_description m_option_descriptions;
  config_map_t                                m_options;
};

}  // namespace SourceXPy

#endif  // SOURCEXTRACTORPLUSPLUS_CONFIGADAPTER_H
