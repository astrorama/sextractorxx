/*
 * CompactSersicModel.h
 *
 *  Created on: Jul 25, 2019
 *      Author: mschefer
 */

#ifndef _MODELFITTING_MODELS_COMPACTSERSICMODEL_H_
#define _MODELFITTING_MODELS_COMPACTSERSICMODEL_H_

#include "ModelFitting/Parameters/BasicParameter.h"
#include "ModelFitting/Models/PositionedModel.h"

#include "SEUtils/Mat22.h"

namespace ModelFitting {

using SExtractor::Mat22;

template <typename ImageType>
class CompactSersicModel : public ExtendedModel<ImageType> {

public:
  CompactSersicModel(BasicParameter& i0, BasicParameter& k, BasicParameter& n,
                BasicParameter& x_scale, BasicParameter& y_scale,
                BasicParameter& rotation, double width, double height,
                BasicParameter& x, BasicParameter& y, std::tuple<double, double, double, double> transform);

  virtual ~CompactSersicModel() = default;

  double getValue(double x, double y) const override;

  ImageType getRasterizedImage(double pixel_scale, std::size_t size_x, std::size_t size_y) const override;

  double evaluateModel(double x, double y) const;
  double samplePixel(int x, int y, unsigned int subsampling) const;
  double adaptiveSamplePixel(int x, int y, unsigned int max_subsampling, double threshold=1.1) const;

protected:

private:
  Mat22 getCombinedTransform(double pixel_scale) const;

  double m_x_scale;
  ReferenceUpdater m_x_scale_updater;

  double m_y_scale;
  ReferenceUpdater m_y_scale_updater;

  double m_rotation;
  ReferenceUpdater m_rotation_updater;

  // Sersic parameters
  double m_i0;
  ReferenceUpdater m_i0_updater;

  double m_k;
  ReferenceUpdater m_k_updater;

  double m_n;
  ReferenceUpdater m_n_updater;

  // Jacobian transform
  Mat22 m_jacobian;
  Mat22 m_inv_jacobian;


  Mat22 m_transform;
};

}

#include "_impl/CompactSersicModel.icpp"

#endif /* _MODELFITTING_MODELS_COMPACTSERSICMODEL_H_ */