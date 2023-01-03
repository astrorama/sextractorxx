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

#include "SEPythonModule/ConfigAdapter.h"
#include <boost/python/extract.hpp>
#include <boost/python/tuple.hpp>

namespace SourceXPy {

namespace py = boost::python;
namespace po = boost::program_options;

ConfigAdapter::ConfigAdapter(po::options_description opt_desc) : m_option_descriptions(std::move(opt_desc)) {
  // Populate defaults
  for (const auto& p : m_option_descriptions.options()) {
    boost::any default_value;
    if (p->semantic()->apply_default(default_value)) {
      m_options[p->long_name()] = boost::program_options::variable_value(default_value, true);
    }
  }
}

void ConfigAdapter::fromPython(const py::dict& config) {
  auto pairs = config.items();
  for (ssize_t i = 0; i < len(pairs); ++i) {
    py::tuple   pair(pairs[i]);
    std::string key = py::extract<std::string>(pair[0]);
    // Dirty trick, serialize into string and let boost options parse back to whatever is expected
    // This is necessary because we can not easily recover the actual expected type unless there is
    // a default, and we want to work also in corner cases as when an integer-like or double is used
    // to setup a float
    std::string value = py::extract<std::string>(pair[1].attr("__str__")());

    const auto opt = m_option_descriptions.find_nothrow(key, false);
    if (opt) {
      boost::any opt_value;
      opt->semantic()->parse(opt_value, {value}, true);
      m_options[opt->long_name()] = boost::program_options::variable_value(opt_value, false);
    }
  }
}

auto ConfigAdapter::getOptions() const -> const config_map_t& {
  return m_options;
}

}  // namespace SourceXPy
