# -*- coding: utf-8 -*-

# Copyright © 2019-2022 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
#
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 3.0 of the License, or (at your option)
# any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
import sys

import _SourceXtractorPy as cpp

from .measurement_config import MeasurementConfig, global_measurement_config
from .model_fitting import ModelFitting

Aperture = cpp.Aperture


def _compat_doc_helper(copy_doc_from):
    """
    Decorate a method setting its docstring to the one from `copy_doc_from`
    """

    def _decorate(func):
        func.__doc__ = copy_doc_from.__doc__
        return func

    return _decorate


@_compat_doc_helper(copy_doc_from=MeasurementConfig.add_aperture_photometry)
def add_aperture_photometry(target, apertures):
    global_measurement_config.add_measurement_image(target)
    return global_measurement_config.add_aperture_photometry(target, apertures)


@_compat_doc_helper(copy_doc_from=MeasurementConfig.print_measurement_images)
def print_measurement_images(file=sys.stderr):
    global_measurement_config.print_measurement_images(file)


@_compat_doc_helper(copy_doc_from=MeasurementConfig.load_fits_image)
def load_fits_image(image, psf=None, weight=None, **kwargs):
    return global_measurement_config.load_fits_image(image, psf, weight, **kwargs)


@_compat_doc_helper(copy_doc_from=MeasurementConfig.load_fits_images)
def load_fits_images(images, psfs=None, weights=None, **kwargs):
    return global_measurement_config.load_fits_images(images, psfs, weights, **kwargs)


@_compat_doc_helper(copy_doc_from=MeasurementConfig.load_fits_data_cube)
def load_fits_data_cube(image, psf=None, weight=None, image_cube_hdu=-1,
                        weight_cube_hdu=-1, **kwargs):
    return global_measurement_config.load_fits_data_cube(image, psf, weight, image_cube_hdu,
                                                         weight_cube_hdu, **kwargs)


@_compat_doc_helper(copy_doc_from=MeasurementConfig.print_output_columns)
def print_output_columns(file=sys.stderr):
    global_measurement_config.print_output_columns(file)


@_compat_doc_helper(copy_doc_from=MeasurementConfig.add_output_column)
def add_output_column(name, params):
    global_measurement_config.add_output_column(name, params)


@_compat_doc_helper(copy_doc_from=ModelFitting.add_prior)
def add_prior(param, value, sigma):
    global_measurement_config.model_fitting.add_prior(param, value, sigma)


@_compat_doc_helper(copy_doc_from=ModelFitting.add_model)
def add_model(group, model):
    global_measurement_config.add_measurement_image(group)
    global_measurement_config.model_fitting.add_model(group, model)


@_compat_doc_helper(copy_doc_from=ModelFitting.set_engine)
def set_engine(engine):
    global_measurement_config.model_fitting.set_engine(engine)


@_compat_doc_helper(copy_doc_from=ModelFitting.set_max_iterations)
def set_max_iterations(max_iter):
    global_measurement_config.model_fitting.set_max_iterations(max_iter)


@_compat_doc_helper(copy_doc_from=ModelFitting.set_meta_iteration_stop)
def set_meta_iterations(max_iter):
    global_measurement_config.model_fitting.set_meta_iterations(max_iter)


@_compat_doc_helper(copy_doc_from=ModelFitting.set_meta_iteration_stop)
def set_meta_iteration_stop(stop):
    global_measurement_config.model_fitting.set_meta_iteration_stop(stop)


@_compat_doc_helper(copy_doc_from=ModelFitting.set_deblend_factor)
def set_deblend_factor(factor):
    global_measurement_config.model_fitting.set_deblend_factor(factor)


@_compat_doc_helper(copy_doc_from=ModelFitting.set_modified_chi_squared_scale)
def set_modified_chi_squared_scale(scale):
    global_measurement_config.model_fitting.set_modified_chi_squared_scale(scale)


@_compat_doc_helper(copy_doc_from=ModelFitting.use_iterative_fitting)
def use_iterative_fitting(use):
    global_measurement_config.model_fitting.use_iterative_fitting(use)


@_compat_doc_helper(copy_doc_from=ModelFitting.set_window_type)
def set_window_type(window_type):
    global_measurement_config.model_fitting.set_window_type(window_type)

@_compat_doc_helper(copy_doc_from=ModelFitting.set_ellipse_scale)
def set_ellipse_scale(ellipse_scale):
    global_measurement_config.model_fitting.set_ellipse_scale(ellipse_scale)


