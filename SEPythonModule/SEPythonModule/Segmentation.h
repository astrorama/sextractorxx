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

#ifndef SOURCEXTRACTORPLUSPLUS_SEGMENTATION_H
#define SOURCEXTRACTORPLUSPLUS_SEGMENTATION_H

#include "SEFramework/Pipeline/PipelineStage.h"
#include "SEPythonModule/Context.h"
#include "SEPythonModule/PipelineReceiver.h"
#include <boost/python/dict.hpp>

// Forward declaration
namespace SourceXtractor {
class Segmentation;
}

namespace SourceXPy {

class Segmentation {
public:
  explicit Segmentation(ContextPtr context);

  std::string repr() const;

  void setNextStage(const boost::python::object& callback);

  void call() const;

private:
  ContextPtr                                    m_context;
  std::shared_ptr<SourceXtractor::Segmentation> m_segmentation;
  std::shared_ptr<SourceReceiverIfce>           m_next_stage;
};

}  // namespace SourceXPy

#endif  // SOURCEXTRACTORPLUSPLUS_SEGMENTATION_H
