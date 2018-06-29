/*
 * VariablePsf.h
 *
 *  Created on: Jun 25, 2018
 *      Author: Alejandro Álvarez Ayllón
 */

#ifndef _SEIMPLEMENTATION_PSF_VARIABLEPSF_H_
#define _SEIMPLEMENTATION_PSF_VARIABLEPSF_H_

#include <SEFramework/Image/VectorImage.h>
#include "SEFramework/Property/PropertyHolder.h"

namespace SExtractor {


/**
 * @class VariablePsf
 *
 * @brief
 * Implements a variable PSF using an arbitrary number of components (i.e. X, Y), and degrees.
 *
 * @details
 * It is based on SExtractor logic, so based on a polynomial where the variables are some attributes
 * of a source. For instance, if the components were X and Y, both in the same group, and the degree 2,
 * the polynomial would be
 *
 *      \f$C + X + X^2 + XY + Y + Y^2\f$
 *
 * The coefficients must be given on that order (note that the constant would be the first element)
 *
 * @todo
 * - Convert into a template
 */
class VariablePsf {
public:

  /**
   * A component belongs to one single group, and is scaled before
   * being used:
   *
   * \f$x_i = (V_i - offset) / scale \f$
   */
  struct Component {
    std::string name;
    int group_id;
    double offset, scale;
  };

  /**
   * Constructor
   * @param pixel_scale
   *    Unused by the class itself, but as it is an attribute of a PSF, it is stored
   * @param components
   *    List of components (or variables) to be used by the Variable PSF
   * @param group_degrees
   *    Polynomial degree. Each group has its own degree, so there has to be as many as different group_id
   *    there are on the components
   */
  VariablePsf(double pixel_scale, const std::vector<Component> &components, const std::vector<int> &group_degrees,
    const std::vector<std::shared_ptr<VectorImage<SeFloat>>> &coefficients);

  /**
   * Convenience constructor that initializes the variable PSF with just a constant value
   * (So it is not variable anymore)
   */
  VariablePsf(double pixel_scale, const std::shared_ptr<VectorImage<SeFloat>> &constant);

  /**
   * Destructor
   */
  virtual ~VariablePsf() = default;

  /**
   * @return The width of the PSF
   */
  int getWidth() const;

  /**
   * @return The height of the PSF
   */
  int getHeight() const;

  /**
   * @return The pixel scale, as passed to the constructor
   */
  double getPixelScale() const;

  /**
   * @return A reference to the list of components
   */
  const std::vector<Component>& getComponents() const;

  /**
   * Reconstructs a PSF based on the given values for each of the component.
   * @param values
   *    Component values. Note that they have to be in the same order (and as many)
   *    as components were passed to the constructor (none for constant PSF).
   * @return
   *    The reconstructed PSF
   * @throws
   *    If the number of values does not match the number of components
   */
  std::shared_ptr<VectorImage<SeFloat>> getPsf(const std::vector<double> &values) const;

private:
  double m_pixel_scale;
  std::vector<Component> m_components;
  std::vector<int> m_group_degrees;
  std::vector<std::shared_ptr<VectorImage<SeFloat>>> m_coefficients;
  std::vector<std::vector<int>> m_exponents;

  /// Verify that the preconditions of getPsf are met at construction time
  void selfTest();

  /// Normalizes the values
  std::vector<double> scaleProperties(const std::vector<double> &values) const;

  /**
   * Calculates the exponents for each component per term of the polynomial.
   * @details
   *    For instance, for (X, Y) degree 2, this would generate the matrix
   *    \code{.unparsed}
   *    [0, 0] // constant
   *    [1, 0] // X
   *    [2, 0] // X^2
   *    [1, 1] // XY
   *    [0, 1] // Y
   *    [0, 2] // Y^2
   *    \endcode
   */
  void calculateExponents();
};

}

#endif //_SEIMPLEMENTATION_PSF_VARIABLEPSF_H_
