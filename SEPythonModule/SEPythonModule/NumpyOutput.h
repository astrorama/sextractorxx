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

#ifndef SOURCEXTRACTORPLUSPLUS_NUMPYOUTPUT_H
#define SOURCEXTRACTORPLUSPLUS_NUMPYOUTPUT_H

#include "SEFramework/Pipeline/PipelineStage.h"
#include "SEFramework/Source/SourceGroupInterface.h"
#include "SEPythonModule/Context.h"
#include <AlexandriaKernel/Semaphore.h>
#include <Table/Row.h>

namespace SourceXPy {

class NumpyOutput : public SourceXtractor::PipelineReceiver<SourceXtractor::SourceGroupInterface> {
public:
  explicit NumpyOutput(ContextPtr context);
  ~NumpyOutput() override;

  std::string repr() const;

  void receiveSource(std::unique_ptr<SourceXtractor::SourceGroupInterface> group) override;

  void receiveProcessSignal(const SourceXtractor::ProcessSourcesEvent& event) override;

  void call(const boost::python::object& obj);

  boost::python::object getTable(std::chrono::microseconds timeout);

private:
  ContextPtr                                                                m_context;
  std::function<Euclid::Table::Row(const SourceXtractor::SourceInterface&)> m_source_to_row;
  std::vector<Euclid::Table::Row>                                           m_rows;
  Euclid::Semaphore                                                         m_semaphore{0};
};

}  // namespace SourceXPy

#endif  // SOURCEXTRACTORPLUSPLUS_NUMPYOUTPUT_H
