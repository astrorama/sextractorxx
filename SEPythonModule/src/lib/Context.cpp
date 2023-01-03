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

#include "SEPythonModule/Context.h"
#include "SEFramework/Output/OutputRegistry.h"
#include "SEFramework/Plugin/PluginManager.h"
#include "SEFramework/Source/SourceGroupWithOnDemandPropertiesFactory.h"
#include "SEFramework/Source/SourceWithOnDemandPropertiesFactory.h"
#include "SEFramework/Task/TaskFactoryRegistry.h"
#include "SEFramework/Task/TaskProvider.h"
#include "SEImplementation/Configuration/DetectionFrameConfig.h"
#include "SEImplementation/Configuration/MultiThreadingConfig.h"
#include "SEImplementation/Configuration/OutputConfig.h"
#include "SEImplementation/Configuration/PythonConfig.h"
#include "SEImplementation/Deblending/DeblendingFactory.h"
#include "SEImplementation/Grouping/GroupingFactory.h"
#include "SEImplementation/Measurement/MeasurementFactory.h"
#include "SEImplementation/Output/OutputFactory.h"
#include "SEImplementation/Partition/PartitionFactory.h"
#include "SEImplementation/Property/RegisterConverters.h"
#include "SEImplementation/Segmentation/SegmentationFactory.h"
#include "SEMain/SourceXtractorConfig.h"
#include "SEPythonModule/ConfigAdapter.h"
#include <Configuration/Configuration.h>
#include <Configuration/Utils.h>
#include <boost/range/algorithm.hpp>

namespace SourceXPy {

namespace Configuration = Euclid::Configuration;
namespace py            = boost::python;
namespace se            = SourceXtractor;
using Euclid::Configuration::ConfigManager;

thread_local std::shared_ptr<Context> Context::s_context = nullptr;

Context::Context(const py::dict& global_config, const py::object& measurement_config)
    : m_task_factory_registry(std::make_shared<se::TaskFactoryRegistry>())
    , m_task_provider(std::make_shared<se::TaskProvider>(m_task_factory_registry))
    , m_output_registry(std::make_shared<se::OutputRegistry>())
    , m_plugin_manager(std::make_shared<se::PluginManager>(m_task_factory_registry, m_output_registry, "",
                                                           std::vector<std::string>{}))
    , m_source_factory(std::make_shared<se::SourceWithOnDemandPropertiesFactory>(m_task_provider))
    , m_group_factory(std::make_shared<se::SourceGroupWithOnDemandPropertiesFactory>(m_task_provider))
    , m_segmentation_factory(std::make_shared<se::SegmentationFactory>(m_task_provider))
    , m_partition_factory(std::make_shared<se::PartitionFactory>(m_source_factory))
    , m_grouping_factory(std::make_shared<se::GroupingFactory>(m_group_factory))
    , m_deblending_factory(std::make_shared<se::DeblendingFactory>(m_source_factory))
    , m_measurement_factory(std::make_shared<se::MeasurementFactory>(m_output_registry))
    , m_output_factory(std::make_shared<se::OutputFactory>(m_output_registry)) {

  // This is *very* risky, but the only alternative to a big refactoring
  m_config_manager.reset(
      &ConfigManager::getInstance(Configuration::getUniqueManagerId()),
      [](const ConfigManager* manager) { Euclid::Configuration::ConfigManager::deregisterInstance(manager->getId()); });

  m_plugin_manager->loadPlugins();

  // Register configuration
  m_segmentation_factory->reportConfigDependencies(*m_config_manager);
  m_partition_factory->reportConfigDependencies(*m_config_manager);
  m_grouping_factory->reportConfigDependencies(*m_config_manager);
  m_deblending_factory->reportConfigDependencies(*m_config_manager);
  m_plugin_manager->getTaskFactoryRegistry().reportConfigDependencies(*m_config_manager);
  m_measurement_factory->reportConfigDependencies(*m_config_manager);
  m_output_factory->reportConfigDependencies(*m_config_manager);

  m_config_manager->registerConfiguration<se::DetectionFrameConfig>();
  m_config_manager->registerConfiguration<se::SourceXtractorConfig>();
  m_config_manager->registerConfiguration<se::OutputConfig>();

  // Configure
  ConfigAdapter config_wrapper(m_config_manager->closeRegistration());
  config_wrapper.fromPython(global_config);
  auto options = config_wrapper.getOptions();

  // Override the python object to use for the measurement configuration and disable output capture
  options["python-config-object"].value()  = boost::any(measurement_config);
  options["python-capture-output"].value() = boost::any(false);

  m_config_manager->initialize(options);

  m_segmentation_factory->configure(*m_config_manager);
  m_partition_factory->configure(*m_config_manager);
  m_grouping_factory->configure(*m_config_manager);
  m_deblending_factory->configure(*m_config_manager);
  m_task_factory_registry->configure(*m_config_manager);
  m_measurement_factory->configure(*m_config_manager);
  m_output_factory->configure(*m_config_manager);

  // Get the thread pool
  m_thread_pool = m_config_manager->getConfiguration<se::MultiThreadingConfig>().getThreadPool();

  // Register the output properties
  m_task_factory_registry->registerPropertyInstances(*m_output_registry);

  // Get hold of the sources-to-row converter
  auto enabled_properties = m_config_manager->getConfiguration<se::OutputConfig>().getOutputProperties();
  m_source_to_row         = m_output_registry->getSourceToRowConverter(enabled_properties);
  se::registerDefaultConverters(*m_output_registry);
}

py::dict Context::get_properties() const {
  py::dict   properties;
  const auto property_names = m_output_registry->getOutputPropertyNames();
  for (auto& prop_name : property_names) {
    const auto column_names = m_output_registry->getColumns(prop_name);
    py::list   attr_names;
    boost::for_each(column_names, [&attr_names](const std::string& c) { attr_names.append(c); });
    properties[prop_name] = attr_names;
  }
  return properties;
}

void Context::check_exception() const {
  m_thread_pool->checkForException(true);
}

void Context::enter(const std::shared_ptr<Context>& context) {
  s_context = context;
}

void Context::exit(const std::shared_ptr<Context>&, [[maybe_unused]] const py::object& exc_type,
                   [[maybe_unused]] const py::object& exc_value, [[maybe_unused]] const py::object& traceback) {
  s_context.reset();
}

const std::shared_ptr<Context>& Context::get_global_context() {
  if (!s_context) {
    PyErr_SetString(PyExc_RuntimeError, "Need an explicit, or an active Context instance");
    py::throw_error_already_set();
  }
  return s_context;
}

}  // namespace SourceXPy
