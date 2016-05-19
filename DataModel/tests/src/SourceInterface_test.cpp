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

#include "DataModel/SourceInterface.h"
#include "DataModel/Property.h"

using namespace DataModel;

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
  MOCK_CONST_METHOD1(getPropertyImpl, Property& (const PropertyId property_id));
};

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (SourceInterface_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( source_interface_test ) {
  MockSourceInterface source_interface;

  ExampleProperty example_property_one(1);
  ExampleProperty example_property_two(2);

  // We expect two calls with different PropertyIds
  EXPECT_CALL(source_interface, getPropertyImpl(PropertyId(typeid(ExampleProperty))))
      .Times(1)
      .WillOnce(ReturnRef(example_property_one));

  EXPECT_CALL(source_interface, getPropertyImpl(PropertyId(typeid(ExampleProperty), "test")))
      .Times(1)
      .WillOnce(ReturnRef(example_property_two));

  // First call it without parameters
  BOOST_CHECK(source_interface.getProperty<ExampleProperty>().m_value == 1);

  // Try the version with a parameter
  BOOST_CHECK(source_interface.getProperty<ExampleProperty>("test").m_value == 2);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END ()
