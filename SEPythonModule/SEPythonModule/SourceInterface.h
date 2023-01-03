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

#ifndef SOURCEXTRACTORPLUSPLUS_SOURCEINTERFACE_H
#define SOURCEXTRACTORPLUSPLUS_SOURCEINTERFACE_H

#include "SEFramework/Source/SourceGroupInterface.h"
#include "SEFramework/Source/SourceInterface.h"
#include "SEImplementation/Property/PixelCoordinateList.h"
#include "SEPythonModule/Context.h"
#include <boost/python/dict.hpp>
#include <boost/python/list.hpp>
#include <boost/python/object.hpp>
#include <memory>
#include <string>

namespace SourceXPy {

/**
 * A DetachedSource is "outside" sourcextractor++'s pipeline, so it does not keep any
 * reference to internal properties such as the detection frame
 */
struct DetachedSource {
  boost::python::dict m_attributes;

  std::string           repr() const;
  boost::python::list   attributes() const;
  boost::python::object attribute(const std::string& key) const;
};

/**
 * An AttachedSource is bound to sourcextractor++'s pipeline. It can not be serialized.
 */
struct AttachedSource {
  ContextPtr                       m_context;
  SourceXtractor::SourceInterface* m_source_ptr = nullptr;

  AttachedSource(ContextPtr context, SourceXtractor::SourceInterface* source_ptr)
      : m_context(std::move(context)), m_source_ptr(source_ptr) {}

  boost::python::object attribute(const std::string& key) const;
  DetachedSource        detach() const;
};

/**
 * An OwnedSource is fully owned by the pipeline stage that receives it.
 * It is still attached to the pipeline, but it is safe to keep a reference to it from Python
 * @warning This is only true since the pipelines clone the sources that come from Python, which is an inefficiency.
 *          If acceptable, m_owned_source could be moved away, m_source_ptr set to nullptr, and let any later call
 *          catch this nullptr situation if the caller kept a reference without explicitly cloning.
 */
struct OwnedSource : public AttachedSource {
  OwnedSource(ContextPtr context, std::unique_ptr<SourceXtractor::SourceInterface> source)
      : AttachedSource(std::move(context), source.get()), m_owned_source(std::move(source)) {}

  std::string repr() const;

  std::unique_ptr<SourceXtractor::SourceInterface> m_owned_source;

  static std::shared_ptr<OwnedSource> create(const std::shared_ptr<Context>& context, int detection_frame_idx,
                                             int detection_id, const boost::python::tuple& pixels);
};

/**
 * An EntangledSource lifetime is bound to its containing SourceGroup. When iterating a SourceGroup,
 * only the current EntangledSource is safe to use. i.e.
 *
 * sources = []
 * for source in group:
 *      print(source)           # Safe
 *      sources.append(source)  # Unsafe
 * print(sources)               # Unsafe
 */
struct EntangledSource : public AttachedSource {
  explicit EntangledSource(ContextPtr context) : AttachedSource(std::move(context), nullptr) {}

  std::string repr() const;
};

/**
 * A SourceGroup is always owned by the called pipeline stage
 */
struct SourceGroup {
  struct Iterator {
    SourceXtractor::SourceGroupInterface::const_iterator m_i;
    SourceXtractor::SourceGroupInterface::const_iterator m_end;
    std::shared_ptr<EntangledSource>                     m_holder;

    // It always returns the same!
    std::shared_ptr<EntangledSource> next();
  };

  std::unique_ptr<SourceXtractor::SourceGroupInterface> m_group;
  ContextPtr                                            m_context;

  std::string           repr() const;
  size_t                size() const;
  boost::python::object attribute(const std::string& key) const;
  Iterator              iter() const;

  static std::shared_ptr<SourceGroup> create(const std::shared_ptr<Context>& context, boost::python::list& sources);
};

}  // namespace SourceXPy

#endif  // SOURCEXTRACTORPLUSPLUS_SOURCEINTERFACE_H
