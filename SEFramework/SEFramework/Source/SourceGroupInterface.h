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
 * @file SourceGroupInterface.h
 * @author nikoapos
 */

#ifndef _SEFRAMEWORK_SOURCEGROUPINTERFACE_H
#define _SEFRAMEWORK_SOURCEGROUPINTERFACE_H

#include "SEFramework/Source/SourceInterface.h"
#include "AlexandriaKernel/memory_tools.h"

namespace SourceXtractor {

/**
 * @class SourceGroupInterface
 * @brief Defines the interface used to group sources
 *
 * @details Warning: please note that the order in which the sources are added to a group is not preserved.
 *
 */

class SourceGroupInterface : public SourceInterface {

  template <typename Collection>
  using CollectionType = typename std::iterator_traits<typename Collection::iterator>::value_type;

public:

  class SourceWrapper : public SourceInterface {
  public:

    explicit SourceWrapper(std::unique_ptr<SourceInterface> source) : m_source(std::move(source)) {}

    SourceWrapper(const SourceWrapper& source) = delete;

    SourceWrapper(SourceWrapper&&) = default;

    const Property& getProperty(const PropertyId& property_id) const override {
      return m_source->getProperty(property_id);
    }

    void setProperty(std::shared_ptr<Property> property, const PropertyId& property_id) override {
      m_source->setProperty(std::move(property), property_id);
    }

    void visitProperties(const PropertyVisitor& visitor) override {
      m_source->visitProperties(visitor);
    }

    std::unique_ptr<SourceInterface> clone() const override {
      return Euclid::make_unique<SourceWrapper>(m_source->clone());
    }

    bool operator<(const SourceWrapper& other) const {
      return this->m_source < other.m_source;
    }

    SourceInterface& getRef() const {
      return *m_source;
    }

    using SourceInterface::getProperty;
    using SourceInterface::setProperty;
    using SourceInterface::setIndexedProperty;

  private:
    std::unique_ptr<SourceInterface> m_source;
  };

  using iterator = std::list<SourceWrapper>::iterator;
  using const_iterator = std::list<SourceWrapper>::const_iterator;

  virtual iterator begin() = 0;
  virtual iterator end() = 0;
  virtual const_iterator cbegin() const = 0;
  virtual const_iterator cend() const = 0;
  virtual const_iterator begin() const = 0;
  virtual const_iterator end() const = 0;

  virtual void addSource(std::unique_ptr<SourceInterface> source) = 0;
  virtual iterator removeSource(iterator pos) = 0;
  virtual void merge(SourceGroupInterface&& other) = 0;
  virtual unsigned int size() const = 0;

  /// Convenient method to add all the sources of a collection
  template <typename SourceCollection>
  void addAllSources(SourceCollection&& sources) {
    static_assert(std::is_base_of<SourceInterface, typename CollectionType<SourceCollection>::element_type>::value,
        "SourceCollection must be a collection of std::shared_ptr to SourceInterface or a type that inherits from it");
    for (auto& source : sources) {
      addSource(std::move(source));
    }
    sources.clear();
  }

  // We introduce the get/setProperty methods from the SourceInterface in the
  // public symbols so they become part of the SourceGroupInterface. The group
  // implementations must implement the methods with the PropertyId
  // in their signature.
  using SourceInterface::getProperty;
  using SourceInterface::setProperty;
  using SourceInterface::setIndexedProperty;
  using SourceInterface::clone;

}; // end of SourceGroupInterface class

} /* namespace SourceXtractor */

#endif /* _SEFRAMEWORK_SOURCEGROUPINTERFACE_H */

