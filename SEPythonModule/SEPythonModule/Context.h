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

#ifndef SOURCEXTRACTORPLUSPLUS_CONTEXT_H
#define SOURCEXTRACTORPLUSPLUS_CONTEXT_H

#include <AlexandriaKernel/ThreadPool.h>
#include <Table/Row.h>
#include <boost/optional.hpp>
#include <boost/python/dict.hpp>

namespace Euclid {
namespace Configuration {
class ConfigManager;
}
}  // namespace Euclid

namespace SourceXtractor {
class TaskFactoryRegistry;
class TaskProvider;
class OutputRegistry;
class PluginManager;
class SegmentationFactory;
class SourceFactory;
class SourceGroupFactory;
class PartitionFactory;
class GroupingFactory;
class DeblendingFactory;
class OutputFactory;
class SourceWithOnDemandPropertiesFactory;
class MeasurementFactory;
class SourceInterface;
}  // namespace SourceXtractor

namespace SourceXPy {

/**
 * Wrap the required context for a sourcextractor++ run
 */
class Context {
public:
  Context(const boost::python::dict& global_config, const boost::python::object& measurement_config);
  Context(const Context&) = delete;
  Context(Context&&)      = default;

  boost::python::dict get_properties() const;

  std::shared_ptr<Euclid::Configuration::ConfigManager>                     m_config_manager;
  std::shared_ptr<SourceXtractor::TaskFactoryRegistry>                      m_task_factory_registry;
  std::shared_ptr<SourceXtractor::TaskProvider>                             m_task_provider;
  std::shared_ptr<SourceXtractor::OutputRegistry>                           m_output_registry;
  std::shared_ptr<SourceXtractor::PluginManager>                            m_plugin_manager;
  std::shared_ptr<SourceXtractor::SourceFactory>                            m_source_factory;
  std::shared_ptr<SourceXtractor::SourceGroupFactory>                       m_group_factory;
  std::shared_ptr<SourceXtractor::SegmentationFactory>                      m_segmentation_factory;
  std::shared_ptr<SourceXtractor::PartitionFactory>                         m_partition_factory;
  std::shared_ptr<SourceXtractor::GroupingFactory>                          m_grouping_factory;
  std::shared_ptr<SourceXtractor::DeblendingFactory>                        m_deblending_factory;
  std::shared_ptr<SourceXtractor::MeasurementFactory>                       m_measurement_factory;
  std::shared_ptr<SourceXtractor::OutputFactory>                            m_output_factory;
  std::function<Euclid::Table::Row(const SourceXtractor::SourceInterface&)> m_source_to_row;
  std::shared_ptr<Euclid::ThreadPool>                                       m_thread_pool;

  /// Rethrow an exception if a worker thread failed
  void check_exception() const;

  static void                            enter(const std::shared_ptr<Context>&);
  static void                            exit(const std::shared_ptr<Context>&, const boost::python::object& exc_type,
                                              const boost::python::object& exc_value, const boost::python::object& traceback);
  static const std::shared_ptr<Context>& get_global_context();

private:
  static thread_local std::shared_ptr<Context> s_context;
};

class ContextPtr {
public:
  ContextPtr(std::shared_ptr<Context> ptr) : m_ptr(std::move(ptr)) {  // NOLINT: Allow implicit conversion
    if (!m_ptr) {
      m_ptr = Context::get_global_context();
    }
  }

  Context* operator->() const {
    return m_ptr.get();
  }

private:
  std::shared_ptr<Context> m_ptr;
};

}  // namespace SourceXPy

#endif  // SOURCEXTRACTORPLUSPLUS_CONTEXT_H
