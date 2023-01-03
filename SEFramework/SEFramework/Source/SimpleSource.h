/**
 * Copyright © 2019-2022 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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
/*
 * SimpleSource.h
 *
 *  Created on: Aug 9, 2016
 *      Author: mschefer
 */

#ifndef _SEFRAMEWORK_SOURCE_SIMPLESOURCE_H_
#define _SEFRAMEWORK_SOURCE_SIMPLESOURCE_H_

#include "AlexandriaKernel/memory_tools.h"
#include "SEFramework/Property/PropertyHolder.h"
#include "SEFramework/Source/SourceInterface.h"

namespace SourceXtractor {

/**
 * @class SimpleSource
 * @brief A trivial implementation of SourceInterface meant to be used in unit tests
 *
 * @details Unlike SourceWithOnDemandproperties, SimpleSource doesn't do any computation of properties but is
 *          instead just a simple container for properties.
 *
 */

class SimpleSource : public SourceInterface {

public:

  /// Destructor
  virtual ~SimpleSource() = default;

  // removes copy/move constructors and assignment operators
  SimpleSource(const SimpleSource&) = delete;
  SimpleSource& operator=(const SimpleSource&) = delete;
  SimpleSource(SimpleSource&&) = delete;
  SimpleSource& operator=(SimpleSource&&) = delete;

  /// Constructor
  SimpleSource() {}

  std::unique_ptr<SourceInterface> clone() const override {
    auto cloned = Euclid::make_unique<SimpleSource>();
    cloned->m_property_holder.update(m_property_holder);
    return std::move(cloned);
  }

  void visitProperties(const PropertyVisitor& visitor) override {
    std::for_each(
        m_property_holder.begin(), m_property_holder.end(),
        [visitor](const std::pair<PropertyId, std::shared_ptr<Property>>& prop) { visitor(prop.first, prop.second); });
  }

  // Note : Because the get/setProperty() methods of the SourceInterface are
  // templated, the overrides of the non-templated versions will hide them. For
  // this reason it is necessary to re-introduce the templated methods, which is
  // done by the using statements below.
  using SourceInterface::getProperty;
  using SourceInterface::setProperty;

protected:

  // Implementation of SourceInterface
  const Property& getProperty(const PropertyId& property_id) const override {
    return m_property_holder.getProperty(property_id);
  }

  void setProperty(std::shared_ptr<Property> property, const PropertyId& property_id) override {
    m_property_holder.setProperty(std::move(property), property_id);
  }

private:
  PropertyHolder m_property_holder;
};

}

#endif /* _SEFRAMEWORK_SOURCE_SIMPLESOURCE_H_ */
