/**
 * @file src/lib/Configuration/OutputConfig.cpp
 * @date 06/23/16
 * @author mschefer
 */

#include <boost/tokenizer.hpp>

#include "Configuration/ConfigManager.h"
#include "Configuration/Configuration.h"
#include "SEImplementation/Configuration/OutputConfig.h"

using namespace Euclid::Configuration;
namespace po = boost::program_options;

namespace SExtractor {

static const std::string OUTPUT_COLUMNS {"output-columns" };

OutputConfig::OutputConfig(long manager_id) : Configuration(manager_id) {
}

std::map<std::string, Configuration::OptionDescriptionList> OutputConfig::getProgramOptions() {
  return { {"Output columns", {
      {OUTPUT_COLUMNS.c_str(), po::value<std::string>()->required(),
          "Comma-separated list of output columns"},
  }}};
}

void OutputConfig::preInitialize(const UserValues& args) {
  // In pre initialize phase we do a fast check that the use gave at least one
  // column name
  if (args.at(OUTPUT_COLUMNS).as<std::string>().size() == 0) {
    throw Elements::Exception() << OUTPUT_COLUMNS << " option must contain at "
            << "least one column name";
  }
}

void OutputConfig::initialize(const UserValues& args) {
  auto& output_columns_string = args.at(OUTPUT_COLUMNS).as<std::string>();

  boost::char_separator<char> separator(",");
  boost::tokenizer<boost::char_separator<char>> tok(output_columns_string, separator);

  std::copy(tok.begin(), tok.end(), back_inserter(m_output_columns));
}

} // SEImplementation namespace



