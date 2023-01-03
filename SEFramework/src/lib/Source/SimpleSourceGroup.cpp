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
 * @file SimpleSourceGroup.cpp
 * @author nikoapos
 */

#include "SEFramework/Source/SimpleSourceGroup.h"
#include "AlexandriaKernel/memory_tools.h"
#include <algorithm>

using Euclid::make_unique;

namespace SourceXtractor {

SimpleSourceGroup::iterator SimpleSourceGroup::begin() {
  return m_sources.begin();
}

SimpleSourceGroup::iterator SimpleSourceGroup::end() {
  return m_sources.end();
}

SimpleSourceGroup::const_iterator SimpleSourceGroup::cbegin() const {
  return m_sources.cbegin();
}

SimpleSourceGroup::const_iterator SimpleSourceGroup::cend() const {
  return m_sources.cend();
}

SimpleSourceGroup::const_iterator SimpleSourceGroup::begin() const {
  return m_sources.cbegin();
}

SimpleSourceGroup::const_iterator SimpleSourceGroup::end() const {
  return m_sources.cend();
}

void SimpleSourceGroup::addSource(std::unique_ptr<SourceInterface> source) {
  m_sources.emplace_back(SourceWrapper(std::move(source)));
}

SourceGroupInterface::iterator SimpleSourceGroup::removeSource(iterator pos) {
  auto next_iter = m_sources.erase(pos);
  return next_iter;
}

void SimpleSourceGroup::merge(SourceGroupInterface&& other) {
  auto& other_group = dynamic_cast<SimpleSourceGroup&>(other);
  for (auto& source : other_group.m_sources) {
    this->m_sources.emplace_back(std::move(source));
  }
  other_group.m_sources.clear();
  m_property_holder.clear();
}

const Property& SimpleSourceGroup::getProperty(const PropertyId& property_id) const {
  return m_property_holder.getProperty(property_id);
}

void SimpleSourceGroup::setProperty(std::shared_ptr<Property> property, const PropertyId& property_id) {
  m_property_holder.setProperty(std::move(property), property_id);
}

unsigned int SimpleSourceGroup::size() const {
  return m_sources.size();
}

std::unique_ptr<SourceInterface> SimpleSourceGroup::clone() const {
  auto cloned = make_unique<SimpleSourceGroup>();
  for (const auto& src : m_sources) {
    cloned->addSource(src.getRef().clone());
  }
  cloned->m_property_holder.update(m_property_holder);
  return std::unique_ptr<SourceInterface>(std::move(cloned));
}

void SimpleSourceGroup::visitProperties(const PropertyVisitor& visitor) {
  std::for_each(
      m_property_holder.begin(), m_property_holder.end(),
      [visitor](const std::pair<PropertyId, std::shared_ptr<Property>>& prop) { visitor(prop.first, prop.second); });
}

}  // namespace SourceXtractor
