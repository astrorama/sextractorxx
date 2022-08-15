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

#ifndef SOURCEXTRACTORPLUSPLUS_PIPELINERECEIVER_H
#define SOURCEXTRACTORPLUSPLUS_PIPELINERECEIVER_H

#include "Pyston/GIL.h"
#include "SEFramework/Pipeline/PipelineStage.h"
#include "SEFramework/Pipeline/SourceGrouping.h"
#include "SEPythonModule/SourceInterface.h"
#include <boost/python/object.hpp>

namespace SourceXPy {

using SourceReceiverIfce = SourceXtractor::PipelineReceiver<SourceXtractor::SourceInterface>;
using GroupReceiverIfce  = SourceXtractor::PipelineReceiver<SourceXtractor::SourceGroupInterface>;

class ProcessSourcesEvent {
public:
  SourceXtractor::ProcessSourcesEvent m_event;

  std::string repr() const;
};

class AllFramesDone : public SourceXtractor::SelectionCriteria {
public:
  bool mustBeProcessed(const SourceXtractor::SourceInterface& source) const override;
};

class PipelineSourceReceiver : public SourceReceiverIfce {
public:
  explicit PipelineSourceReceiver(boost::python::object callback, ContextPtr context)
      : m_callback(std::move(callback)), m_context(std::move(context)){};

  ~PipelineSourceReceiver() override = default;

  void receiveSource(std::unique_ptr<SourceXtractor::SourceInterface> source) override {
    Pyston::GILLocker gil;
    m_callback(std::make_shared<OwnedSource>(m_context, std::move(source)));
  }

  void receiveProcessSignal(const SourceXtractor::ProcessSourcesEvent& event) override {
    Pyston::GILLocker gil;
    m_callback(ProcessSourcesEvent{event});
  }

private:
  boost::python::object m_callback;
  ContextPtr            m_context;
};

class PipelineGroupReceiver : public GroupReceiverIfce {
public:
  explicit PipelineGroupReceiver(boost::python::object callback, ContextPtr context)
      : m_callback(std::move(callback)), m_context(std::move(context)){};

  ~PipelineGroupReceiver() override = default;

  void receiveSource(std::unique_ptr<SourceXtractor::SourceGroupInterface> source) override {
    Pyston::GILLocker gil;
    m_callback(std::make_shared<SourceGroup>(SourceGroup{std::move(source), m_context}));
  }

  void receiveProcessSignal(const SourceXtractor::ProcessSourcesEvent& event) override {
    Pyston::GILLocker gil;
    m_callback(ProcessSourcesEvent{event});
  }

private:
  boost::python::object m_callback;
  ContextPtr            m_context;
};

}  // namespace SourceXPy

#endif  // SOURCEXTRACTORPLUSPLUS_PIPELINERECEIVER_H