/**
 * @file SigmoidConverter.h
 *
 * Created on: July 28, 2015
 *     Author: Pierre Dubath
 */

#ifndef SIGMOID_CONVERTER_H_
#define SIGMOID_CONVERTER_H_

#include<string>
#include<memory>
#include<math.h>
#include "ModelFitting/Parameters/CoordinateConverter.h"

namespace ModelFitting {

/**
 * @class SigmoidConverter
 * @brief
 *    CoordinateConverter implementation using the sigmoid function
 *
 */
class SigmoidConverter: public CoordinateConverter {

public:

  SigmoidConverter(const double min_value, const double max_value) :
      m_min_value(min_value), m_max_value(max_value) {
    if (m_min_value > m_max_value) {
      throw Elements::Exception()
          << "SigmoidConverter: min_value larger than max_value!";
    }
  }

  /**
   * @brief
   *    World to engine coordinate converter
   *
   * @param world_value
   *    The value of the parameter in world coordinate
   *
   * @return engine_value
   *    The value of the parameter in engine coordinate
   *
   */
  double worldToEngine(const double world_value) const override {
    if (world_value < m_min_value || world_value > m_max_value) {
      throw Elements::Exception()
          << "WorldToEngine SigmoidConverter: world values outside of possible range";
    }
    return log((world_value - m_min_value) / (m_max_value - world_value));
  }

  /**
   * @brief
   *    Engine to world coordinate converter
   *
   * @param engine_value
   *
   * @return world_value
   *
   */
  double engineToWorld(const double engine_value) const override {
    return m_min_value + (m_max_value - m_min_value) / (1 + exp(-engine_value));
  }

  /**
   * @brief Destructor
   */
  virtual ~SigmoidConverter() {
  }

private:

  /// minimum model value in world coordinates
  const double m_min_value;

  /// maximum model value in world coordinates
  const double m_max_value;

};

} // namespace ModelFitting

#endif /* SIGMOID_CONVERTER_H_ */
