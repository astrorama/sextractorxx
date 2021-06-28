/** Copyright © 2021 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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

#include "ModelFitting/Engine/ResidualEstimator.h"
#include "ModelFitting/Engine/AsinhChiSquareComparator.h"
#include "ModelFitting/Engine/DataVsModelResiduals.h"
#include "ModelFitting/Engine/LeastSquareEngineManager.h"

#include "SEImplementation/Image/ImagePsf.h"
#include "SEImplementation/Image/VectorImageDataVsModelInputTraits.h"

#include "SEImplementation/CheckImages/CheckImages.h"

#include "SEImplementation/Plugin/DetectionFrameCoordinates/DetectionFrameCoordinates.h"
#include "SEImplementation/Plugin/MeasurementFrameRectangle/MeasurementFrameRectangle.h"
#include "SEImplementation/Plugin/MeasurementFrameImages/MeasurementFrameImages.h"
#include "SEImplementation/Plugin/MeasurementFrameInfo/MeasurementFrameInfo.h"
#include "SEImplementation/Plugin/MeasurementFrameCoordinates/MeasurementFrameCoordinates.h"
#include "SEImplementation/Plugin/Jacobian/Jacobian.h"
#include "SEImplementation/Plugin/Psf/PsfProperty.h"

#include "SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFitting.h"
#include "SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingParameterManager.h"
#include "SEImplementation/Plugin/FlexibleModelFitting/FlexibleModelFittingIterativeTask.h"

namespace SourceXtractor {

using namespace ModelFitting;

static auto logger = Elements::Logging::getLogger("FlexibleModelFitting");

FlexibleModelFittingIterativeTask::FlexibleModelFittingIterativeTask(const std::string &least_squares_engine,
    unsigned int max_iterations, double modified_chi_squared_scale,
    std::vector<std::shared_ptr<FlexibleModelFittingParameter>> parameters,
    std::vector<std::shared_ptr<FlexibleModelFittingFrame>> frames,
    std::vector<std::shared_ptr<FlexibleModelFittingPrior>> priors,
    double scale_factor)
  : m_least_squares_engine(least_squares_engine),
    m_max_iterations(max_iterations), m_modified_chi_squared_scale(modified_chi_squared_scale),
    m_parameters(parameters), m_frames(frames), m_priors(priors), m_scale_factor(scale_factor) {}

FlexibleModelFittingIterativeTask::~FlexibleModelFittingIterativeTask() {
}


namespace {

PixelRectangle getFittingRect(SourceInterface& source, int frame_index) {
  return source.getProperty<MeasurementFrameRectangle>(frame_index).getRect(); //FIXME add border
}

bool isFrameValid(SourceInterface& source, int frame_index) {
  auto stamp_rect = getFittingRect(source, frame_index);
  return stamp_rect.getWidth() > 0 && stamp_rect.getHeight() > 0;
}

std::shared_ptr<VectorImage<SeFloat>> createImageCopy(SourceInterface& source, int frame_index) {
  const auto& frame_images = source.getProperty<MeasurementFrameImages>(frame_index);
  auto rect = getFittingRect(source, frame_index);

  auto image = VectorImage<SeFloat>::create(frame_images.getImageChunk(
      LayerSubtractedImage, rect.getTopLeft().m_x, rect.getTopLeft().m_y, rect.getWidth(), rect.getHeight()));

  return image;
}

std::shared_ptr<VectorImage<SeFloat>> createWeightImage(SourceInterface& source, int frame_index) {
  const auto& frame_images = source.getProperty<MeasurementFrameImages>(frame_index);
  auto frame_image = frame_images.getLockedImage(LayerSubtractedImage);
  auto frame_image_thresholded = frame_images.getLockedImage(LayerThresholdedImage);
  auto variance_map = frame_images.getLockedImage(LayerVarianceMap);

  const auto& frame_info = source.getProperty<MeasurementFrameInfo>(frame_index);
  SeFloat gain = frame_info.getGain();
  SeFloat saturation = frame_info.getSaturation();

  auto rect = getFittingRect(source, frame_index);
  auto weight = VectorImage<SeFloat>::create(rect.getWidth(), rect.getHeight());

  for (int y = 0; y < rect.getHeight(); y++) {
    for (int x = 0; x < rect.getWidth(); x++) {
      auto back_var = variance_map->getValue(rect.getTopLeft().m_x + x, rect.getTopLeft().m_y + y);
      auto pixel_val = frame_image->getValue(rect.getTopLeft().m_x + x, rect.getTopLeft().m_y + y);
      if (saturation > 0 && pixel_val > saturation) {
        weight->at(x, y) = 0;
      }
      else if (gain > 0.0 && pixel_val > 0.0) {
        weight->at(x, y) = sqrt(1.0 / (back_var + pixel_val / gain));
      }
      else {
        weight->at(x, y) = sqrt(1.0 / back_var); // infinite gain
      }
    }
  }

  return weight;
}
FrameModel<ImagePsf, std::shared_ptr<VectorImage<SourceXtractor::SeFloat>>> createFrameModel(
    SourceInterface& source, double pixel_scale, FlexibleModelFittingParameterManager& manager,
    std::shared_ptr<FlexibleModelFittingFrame> frame, PixelRectangle stamp_rect) {

  int frame_index = frame->getFrameNb();

  auto frame_coordinates = source.getProperty<MeasurementFrameCoordinates>(frame_index).getCoordinateSystem();
  auto ref_coordinates = source.getProperty<DetectionFrameCoordinates>().getCoordinateSystem();

  // FIXME: for now still use the group's PSF property, replace by a PSF for the source
  auto psf_property = source.getProperty<PsfProperty>(frame_index);
  auto jacobian = source.getProperty<JacobianSource>(frame_index).asTuple();

  // The model fitting module expects to get a PSF with a pixel scale, but we have the pixel sampling step size
  // It will be used to compute the rastering grid size, and after convolving with the PSF the result will be
  // downscaled before copied into the frame image.
  // We can multiply here then, as the unit is pixel/pixel, rather than "/pixel or similar
  auto group_psf = ImagePsf(pixel_scale * psf_property.getPixelSampling(), psf_property.getPsf());

  std::vector<ConstantModel> constant_models;
  std::vector<PointModel> point_models;
  std::vector<std::shared_ptr<ModelFitting::ExtendedModel<ImageInterfaceTypePtr>>> extended_models;

  for (auto model : frame->getModels()) {
    model->addForSource(manager, source, constant_models, point_models, extended_models, jacobian, ref_coordinates,
        frame_coordinates, stamp_rect.getTopLeft());
  }

  // Full frame model with all sources
  FrameModel<ImagePsf, std::shared_ptr<VectorImage<SourceXtractor::SeFloat>>> frame_model(
    pixel_scale, (size_t) stamp_rect.getWidth(), (size_t) stamp_rect.getHeight(),
    std::move(constant_models), std::move(point_models), std::move(extended_models), group_psf);

  return frame_model;
}

}


void FlexibleModelFittingIterativeTask::computeProperties(SourceGroupInterface& group) const {
  FittingState fitting_state;

  for (auto& source : group) {
    SourceState initial_state;
    initial_state.iterations = 0;
    initial_state.stop_reason = 0;
    initial_state.reduced_chi_squared = 0.0;

    for (auto parameter : m_parameters) {
      auto free_parameter = std::dynamic_pointer_cast<FlexibleModelFittingFreeParameter>(parameter);
      if (free_parameter != nullptr) {
        initial_state.parameters_values[free_parameter->getId()] = free_parameter->getInitialValue(source);
      } else {
        initial_state.parameters_values[parameter->getId()] = 0.0;
      }
    }
    fitting_state.source_states.emplace_back(std::move(initial_state));
  }

  for (int iteration = 0; iteration<3; iteration++) {
    int index = 0;
    for (auto& source : group) {
      fitSource(group, source, index, fitting_state);
      index++;
    }
  }

  int index = 0;
  for (auto& source : group) {
    auto& source_state = fitting_state.source_states.at(index);
    // FIXME not complete
    source.setProperty<FlexibleModelFitting>(source_state.iterations, source_state.stop_reason,
        source_state.reduced_chi_squared, Flags::NONE,
        source_state.parameters_values, source_state.parameters_sigmas);

    index++;
  }
}


// Used to set a dummy property in case of error that contains no result but just an error flag
void FlexibleModelFittingIterativeTask::setDummyProperty(SourceInterface& source, Flags flags) const {
  std::unordered_map<int, double> dummy_values;
  for (auto parameter : m_parameters) {
    dummy_values[parameter->getId()] = std::numeric_limits<double>::quiet_NaN();
  }
  source.setProperty<FlexibleModelFitting>(0, 0, std::numeric_limits<double>::quiet_NaN(), flags,
                                           dummy_values, dummy_values);
}

std::shared_ptr<VectorImage<SeFloat>> FlexibleModelFittingIterativeTask::createDeblendImage(
    SourceGroupInterface& group, SourceInterface& source, int source_index,
    std::shared_ptr<FlexibleModelFittingFrame> frame, FittingState& state) const {
  int frame_index = frame->getFrameNb();
  auto rect = getFittingRect(source, frame_index);

  double pixel_scale = 1 / m_scale_factor;
  FlexibleModelFittingParameterManager parameter_manager;
  ModelFitting::EngineParameterManager engine_parameter_manager {};
  int n_free_parameters = 0;

  int index = 0;
  for (auto& src : group) {
//    if (index != source_index) {
     for (auto parameter : m_parameters) {
        auto free_parameter = std::dynamic_pointer_cast<FlexibleModelFittingFreeParameter>(parameter);

        if (free_parameter != nullptr) {
          ++n_free_parameters;

          // Initial with the values from the current iteration run
          parameter_manager.addParameter(src, parameter,
              free_parameter->create(parameter_manager, engine_parameter_manager, src,
                  state.source_states[index].parameters_values.at(free_parameter->getId())));

        } else {
          parameter_manager.addParameter(src, parameter,
              parameter->create(parameter_manager, engine_parameter_manager, src));
        }
      }
//    }
    index++;
  }

  auto deblend_image = VectorImage<SeFloat>::create(rect.getWidth(), rect.getHeight());
  for (auto& src : group) {
    if (index != source_index) {
        auto frame_model = createFrameModel(src, pixel_scale, parameter_manager, frame, rect);
        auto final_stamp = frame_model.getImage();

        for (int y = 0; y < final_stamp->getHeight(); ++y) {
          for (int x = 0; x < final_stamp->getWidth(); ++x) {
            deblend_image->at(x, y) += final_stamp->at(x, y);
          }
        }
    }
  }

  return deblend_image;
}

void FlexibleModelFittingIterativeTask::fitSource(SourceGroupInterface& group, SourceInterface& source, int index, FittingState& state) const {
  double pixel_scale = 1 / m_scale_factor;

  //////////////////////////////////////////////
  // Prepare parameters

  FlexibleModelFittingParameterManager parameter_manager;
  ModelFitting::EngineParameterManager engine_parameter_manager{};
  int n_free_parameters = 0;

  for (auto parameter : m_parameters) {
    auto free_parameter = std::dynamic_pointer_cast<FlexibleModelFittingFreeParameter>(parameter);

    if (free_parameter != nullptr) {
      ++n_free_parameters;

      // Initial with the values from the current iteration run
      parameter_manager.addParameter(source, parameter,
          free_parameter->create(parameter_manager, engine_parameter_manager, source,
              state.source_states[index].parameters_values.at(free_parameter->getId())));

    } else {
      parameter_manager.addParameter(source, parameter,
          parameter->create(parameter_manager, engine_parameter_manager, source));
    }

  }

  // Reset access checks, as a dependent parameter could have triggered it
  parameter_manager.clearAccessCheck();


  ///////////////////////////////////////////////////////////////////////////////////
  // Add models for all frames
  ResidualEstimator res_estimator {};

  int valid_frames = 0;
  int n_good_pixels = 0;
  for (auto frame : m_frames) {
    int frame_index = frame->getFrameNb();
    // Validate that each frame covers the model fitting region
    if (isFrameValid(source, frame_index)) {
      valid_frames++;

      auto stamp_rect = getFittingRect(source, frame_index);
      auto frame_model = createFrameModel(source, pixel_scale, parameter_manager, frame, stamp_rect);

      auto image = createImageCopy(source, frame_index);

      auto deblend_image = createDeblendImage(group, source, index, frame, state);
      for (int y = 0; y < image->getHeight(); ++y) {
        for (int x = 0; x < image->getWidth(); ++x) {
          image->at(x, y) -= deblend_image->at(x, y);
        }
      }

      auto weight = createWeightImage(source, frame_index);

      // count number of pixels that can be used for fitting
      for (int y = 0; y < weight->getHeight(); ++y) {
        for (int x = 0; x < weight->getWidth(); ++x) {
          n_good_pixels += (weight->at(x, y) != 0.);
        }
      }

      // Setup residuals
      auto data_vs_model =
        createDataVsModelResiduals(image, std::move(frame_model), weight,
                                   //LogChiSquareComparator(m_modified_chi_squared_scale));
                                   AsinhChiSquareComparator(m_modified_chi_squared_scale));
      res_estimator.registerBlockProvider(std::move(data_vs_model));
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Check that we had enough data for the fit
  Flags group_flags = Flags::NONE;
  if (valid_frames == 0) {
    group_flags = Flags::OUTSIDE;
  }
  else if (n_good_pixels < n_free_parameters) {
    group_flags = Flags::INSUFFICIENT_DATA;
  }

  if (group_flags != Flags::NONE) {
    setDummyProperty(source, group_flags);
    return;
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Add priors
  for (auto prior : m_priors) {
    prior->setupPrior(parameter_manager, source, res_estimator);
  }

  /////////////////////////////////////////////////////////////////////////////////
  // Model fitting

  auto engine = LeastSquareEngineManager::create(m_least_squares_engine, m_max_iterations);
  auto solution = engine->solveProblem(engine_parameter_manager, res_estimator);

  auto iterations = solution.iteration_no;
  auto stop_reason = solution.engine_stop_reason;
  if (solution.status_flag == LeastSquareSummary::ERROR) {
    group_flags |= Flags::ERROR;
  }


  ////////////////////////////////////////////////////////////////////////////////////
  // compute chi squared

  int total_data_points = 0;
  SeFloat avg_reduced_chi_squared = computeChiSquared(source, pixel_scale, parameter_manager, total_data_points);

  int nb_of_free_parameters = 0;
  for (auto parameter : m_parameters) {
    bool is_free_parameter = std::dynamic_pointer_cast<FlexibleModelFittingFreeParameter>(parameter).get();
    bool accessed_by_modelfitting = parameter_manager.isParamAccessed(source, parameter);
    if (is_free_parameter && accessed_by_modelfitting) {
      nb_of_free_parameters++;
    }
  }
  avg_reduced_chi_squared /= (total_data_points - nb_of_free_parameters);

  ////////////////////////////////////////////////////////////////////////////////////
  // Collect parameters for output
  std::unordered_map<int, double> parameter_values, parameter_sigmas;
  auto source_flags = group_flags;

  for (auto parameter : m_parameters) {
    bool is_dependent_parameter = std::dynamic_pointer_cast<FlexibleModelFittingDependentParameter>(parameter).get();
    bool accessed_by_modelfitting = parameter_manager.isParamAccessed(source, parameter);
    auto modelfitting_parameter = parameter_manager.getParameter(source, parameter);

    if (is_dependent_parameter || accessed_by_modelfitting) {
      parameter_values[parameter->getId()] = modelfitting_parameter->getValue();
      parameter_sigmas[parameter->getId()] = parameter->getSigma(parameter_manager, source, solution.parameter_sigmas);
    } else {
      parameter_values[parameter->getId()] = state.source_states[index].parameters_values[parameter->getId()];
      parameter_sigmas[parameter->getId()] = state.source_states[index].parameters_sigmas[parameter->getId()];

//      // Need to cascade the NaN to any potential dependent parameter
//      auto engine_parameter = std::dynamic_pointer_cast<EngineParameter>(modelfitting_parameter);
//      if (engine_parameter) {
//        engine_parameter->setEngineValue(std::numeric_limits<double>::quiet_NaN());
//      }
//      parameter_values[parameter->getId()] = std::numeric_limits<double>::quiet_NaN();
//      parameter_sigmas[parameter->getId()] = std::numeric_limits<double>::quiet_NaN();
//      source_flags |= Flags::PARTIAL_FIT;
    }
  }

  state.source_states[index].parameters_values = parameter_values;
  state.source_states[index].parameters_sigmas = parameter_sigmas;
  state.source_states[index].reduced_chi_squared = avg_reduced_chi_squared;
  state.source_states[index].iterations = iterations;
  state.source_states[index].stop_reason = stop_reason;

  ////////////////////////////////////////////////////////////////////
  // Check images

  updateCheckImages(source, pixel_scale, parameter_manager);
}

  void FlexibleModelFittingIterativeTask::updateCheckImages(SourceInterface& source,
    double pixel_scale, FlexibleModelFittingParameterManager& manager) const {

  int frame_id = 0;
  for (auto frame : m_frames) {
    int frame_index = frame->getFrameNb();
    // Validate that each frame covers the model fitting region
    if (isFrameValid(source, frame_index)) {
      auto stamp_rect = getFittingRect(source, frame_index);
      auto frame_model = createFrameModel(source, pixel_scale, manager, frame, stamp_rect);
      auto final_stamp = frame_model.getImage();

      auto debug_image = CheckImages::getInstance().getModelFittingImage(frame_index);
      if (debug_image) {
        ImageAccessor<SeFloat> debugAccessor(debug_image);
        for (int x = 0; x < final_stamp->getWidth(); x++) {
          for (int y = 0; y < final_stamp->getHeight(); y++) {
            auto x_coord = stamp_rect.getTopLeft().m_x + x;
            auto y_coord = stamp_rect.getTopLeft().m_y + y;
            debug_image->setValue(x_coord, y_coord,
                                  debugAccessor.getValue(x_coord, y_coord) + final_stamp->getValue(x, y));
          }
        }
      }
    }
    frame_id++;
  }
}


SeFloat FlexibleModelFittingIterativeTask::computeChiSquaredForFrame(std::shared_ptr<const Image<SeFloat>> image,
    std::shared_ptr<const Image<SeFloat>> model, std::shared_ptr<const Image<SeFloat>> weights, int& data_points) const {
  double reduced_chi_squared = 0.0;
  data_points = 0;

  ImageAccessor<SeFloat> imageAccessor(image), modelAccessor(model);
  ImageAccessor<SeFloat> weightAccessor(weights);

  for (int y=0; y < image->getHeight(); y++) {
    for (int x=0; x < image->getWidth(); x++) {
      double tmp = imageAccessor.getValue(x, y) - modelAccessor.getValue(x, y);
      reduced_chi_squared += tmp * tmp * weightAccessor.getValue(x, y) * weightAccessor.getValue(x, y);
      if (weightAccessor.getValue(x, y) > 0) {
        data_points++;
      }
    }
  }
  return reduced_chi_squared;
}


SeFloat FlexibleModelFittingIterativeTask::computeChiSquared(SourceInterface& source,
    double pixel_scale, FlexibleModelFittingParameterManager& manager, int& total_data_points) const {
  SeFloat total_chi_squared = 0;
  total_data_points = 0;
  int valid_frames = 0;
  for (auto frame : m_frames) {
    int frame_index = frame->getFrameNb();
    // Validate that each frame covers the model fitting region
    if (isFrameValid(source, frame_index)) {
      valid_frames++;
      auto stamp_rect = getFittingRect(source, frame_index);
      auto frame_model = createFrameModel(source, pixel_scale, manager, frame, stamp_rect);
      auto final_stamp = frame_model.getImage();

      auto image = createImageCopy(source, frame_index);
      auto weight = createWeightImage(source, frame_index);

      int data_points = 0;
      SeFloat chi_squared = computeChiSquaredForFrame(
          image, final_stamp, weight, data_points);

      total_data_points += data_points;
      total_chi_squared += chi_squared;
    }
  }

  return total_chi_squared;
}

}


