#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include "SEMain/ProgressReporterConfiguration.h"

using Euclid::Configuration::Configuration;
namespace po = boost::program_options;

namespace SExtractor {

static const std::string PROGRESS_MIN_INTERVAL{"progress-min-interval"};
static const std::string PROGRESS_BAR_DISABLED{"progress-bar-disable"};

ProgressReporterConfiguration::ProgressReporterConfiguration(long manager_id): Configuration(manager_id) {
}

std::map<std::string, Configuration::OptionDescriptionList> ProgressReporterConfiguration::getProgramOptions() {
  return {
    {"Progress feedback",
      {
        {PROGRESS_MIN_INTERVAL.c_str(), po::value<int>()->default_value(5),
          "Minimal interval to wait before printing a new log entry with the progress report"},
        {PROGRESS_BAR_DISABLED.c_str(), po::bool_switch(),
          "Disable progress bar display"}
      }
    }
  };
}

void ProgressReporterConfiguration::preInitialize(const UserValues&) {}

void ProgressReporterConfiguration::initialize(const Configuration::UserValues& args) {
  auto nseconds = args.at(PROGRESS_MIN_INTERVAL).as<int>();
  m_min_interval = boost::posix_time::seconds(nseconds);
  m_disable_progress_bar = args.at(PROGRESS_BAR_DISABLED).as<bool>();
  m_log_file_set = (args.find("log-file") !=  args.end()) && !args.at("log-file").empty();
}

} // end SExtractor
