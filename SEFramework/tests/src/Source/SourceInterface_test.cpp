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
/**
 * @file tests/src/SourceInterface_test.cpp
 * @date 05/04/16
 * @author mschefer
 */

#include <memory>

#include <boost/test/unit_test.hpp>
#include "ElementsKernel/EnableGMock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

#include "SEFramework/Source/SourceInterface.h"
#include "SEFramework/Property/Property.h"


using namespace SourceXtractor;

// Example Property containing an int
class ExampleProperty : public Property {
public:
  int m_value;

  ExampleProperty(int value) : m_value(value) {}
};

// Mock implementation
// the goal of this test of to verify that the template method correctly calls getPropertyImpl()
class MockSourceInterface : public SourceInterface {
public:
  using SourceInterface::getProperty;
  using SourceInterface::setProperty;

  MOCK_CONST_METHOD1(getProperty, Property& (const PropertyId& property_id));
  void setProperty(std::shared_ptr<Property>, const PropertyId& ) {}

  void visitProperties(const PropertyVisitor&) override {}

  std::unique_ptr<SourceInterface> clone() const override {
    return nullptr;
  }
};

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (SourceInterface_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( source_interface_test ) {
  MockSourceInterface source_interface;

  const int property_index = 3;

  ExampleProperty example_property_one(1);
  ExampleProperty example_property_two(2);

  // We expect two calls with different PropertyIds
  EXPECT_CALL(source_interface, getProperty(PropertyId::create<ExampleProperty>()))
      .Times(1)
      .WillOnce(ReturnRef(example_property_one));

  EXPECT_CALL(source_interface, getProperty(PropertyId::create<ExampleProperty>(property_index)))
      .Times(1)
      .WillOnce(ReturnRef(example_property_two));

  // First call it without parameters
  BOOST_CHECK(source_interface.getProperty<ExampleProperty>().m_value == 1);

  // Try the version with a parameter
  BOOST_CHECK(source_interface.getProperty<ExampleProperty>(property_index).m_value == 2);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END ()
