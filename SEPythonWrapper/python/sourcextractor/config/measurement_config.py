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
from __future__ import division, print_function

import sys

import _SEPythonConfig as cpp

from .measurement_images import DataCubeSlice, FitsFile, ImageGroup, MeasurementGroup, \
    MeasurementImage
from .model_fitting import ModelFitting, ParameterBase


class MeasurementConfig:
    def __init__(self):
        self._measurement_images = {}
        self._apertures_for_image = {}
        self._used_names = set()
        self._model_fitting_parameter_columns = []
        self._aperture_columns = []
        self._column_map = {
            ParameterBase: self._model_fitting_parameter_columns,
            cpp.Aperture: self._aperture_columns
        }
        self._model_fitting = ModelFitting()

    @property
    def apertures_for_image(self):
        return self._apertures_for_image

    @property
    def measurement_images(self):
        return self._measurement_images

    @property
    def model_fitting_parameter_columns(self):
        return self._model_fitting_parameter_columns

    @property
    def aperture_columns(self):
        return self._aperture_columns

    @property
    def model_fitting(self):
        return self._model_fitting

    def add_measurement_image(self, image):
        if isinstance(image, MeasurementImage):
            if image.id not in self._measurement_images:
                self._measurement_images[image.id] = image
        elif image.is_leaf():
            for member in image:
                self.add_measurement_image(member)
        else:
            for _, subgroup in image:
                self.add_measurement_image(subgroup)

    def load_fits_image(self, image, psf=None, weight=None, **kwargs):
        """
        Creates an image group with the images of a (possibly multi-HDU) single FITS file.

        If image is multi-hdu, psf and weight can either be multi hdu or lists of individual files.

        In any case, they are matched in order and HDUs not containing images (two dimensional arrays) are ignored.

        :param image: The filename of the FITS file containing the image(s)
        :param psf: psf file or list of psf files
        :param weight: FITS file for the weight image or a list of such files

        :return: A ImageGroup representing the images
        """

        image_file = FitsFile(image)
        if "image_hdu" in kwargs.keys():
            image_hdu_list = [kwargs.pop("image_hdu")]
        else:
            image_hdu_list = image_file.hdu_list

        # handles the PSFs
        if isinstance(psf, list):
            if len(psf) != len(image_hdu_list):
                raise ValueError("The number of psf files must match the number of images!")
            psf_file_list = psf
            psf_hdu_list = [0] * len(psf_file_list)
        else:
            if "psf_hdu" in kwargs.keys():
                psf_hdu_list = [kwargs.pop("psf_hdu")] * len(image_hdu_list)
            else:
                psf_hdu_list = range(len(image_hdu_list))
            psf_file_list = [psf] * len(image_hdu_list)

        # handles the weight maps
        if isinstance(weight, list):
            if len(weight) != len(image_hdu_list):
                raise ValueError("The number of weight files must match the number of images!")
            weight_file_list = weight
            weight_hdu_list = [0] * len(weight_file_list)
        elif weight is None:
            weight_file_list = [None] * len(image_hdu_list)
            weight_hdu_list = [0] * len(weight_file_list)
        else:
            weight_file = FitsFile(weight)
            if "weight_hdu" in kwargs.keys():
                weight_hdu_list = [kwargs.pop("weight_hdu")] * len(image_hdu_list)
            else:
                weight_hdu_list = weight_file.hdu_list
            weight_file_list = [weight_file] * len(image_hdu_list)

        image_list = []
        for hdu, psf_file, psf_hdu, weight_file, weight_hdu in zip(
                image_hdu_list, psf_file_list, psf_hdu_list, weight_file_list, weight_hdu_list):
            image_list.append(MeasurementImage(image_file, psf_file, weight_file,
                                               image_hdu=hdu, psf_hdu=psf_hdu,
                                               weight_hdu=weight_hdu,
                                               **kwargs))
        for img in image_list:
            self._measurement_images[img.id] = img
        return ImageGroup(images=image_list)

    def load_fits_images(self, images, psfs=None, weights=None, **kwargs):
        """Creates an image group for the given images.

        Parameters
        ----------
        images : list of str
            A list of relative paths to the images FITS files. Can also be single string in which case,
             this function acts like load_fits_image
        psfs : list of str
            A list of relative paths to the PSF FITS files (optional). It must match the length of image_list or be None.
        weights : list of str
            A list of relative paths to the weight files (optional). It must match the length of image_list or be None.

        Returns
        -------
        ImageGroup
            A ImageGroup representing the images

        Raises
        ------
        ValueError
            In case of mismatched list of files
        """

        if isinstance(images, list):
            if len(images) == 0:
                raise ValueError("An empty list passed to load_fits_images")

            psfs = psfs or [None] * len(images)
            weights = weights or [None] * len(images)

            if not isinstance(psfs, list) or len(psfs) != len(images):
                raise ValueError("The number of image files and psf files must match!")

            if not isinstance(weights, list) or len(weights) != len(images):
                raise ValueError("The number of image files and weight files must match!")

            groups = []
            for f, p, w in zip(images, psfs, weights):
                groups.append(self.load_fits_image(f, p, w, **kwargs))

            image_list = []
            for g in groups:
                image_list += g
            for img in image_list:
                self._measurement_images[img.id] = img
            return ImageGroup(images=image_list)
        else:
            return self.load_fits_image(images, psfs, weights, **kwargs)

    def load_fits_data_cube(self, image, psf=None, weight=None, image_cube_hdu=-1,
                            weight_cube_hdu=-1, **kwargs):
        """
        Creates an image group with the images of a data cube.

        weight can be a matching datacube, multi-hdu or list of individual images
        psf can be a multi-hdu or list of individual images to be matched

        In any case, they are matched in order and HDUs not containing images are ignored.

        :param image: The filename of the FITS file containing the image datacube
        :param psf: psf file or list of psf files
        :param weight: FITS file contianing a weight datacube, a MEF contianing the weights
             or a list of such files
        :param image_cube_hdu: hdu containing the image datacube, default = first HDU containing image data
        :param weight_cube_hdu: hdu containing the weight datacube, default = first HDU containing image data

        :return: A ImageGroup representing the images
        """

        image_cube_file = FitsFile(image)

        if image_cube_hdu < 0:
            cube_hdu = image_cube_file.hdu_list[0]
        else:
            cube_hdu = image_cube_hdu

        dims = image_cube_file.get_dimensions(cube_hdu)
        if len(dims) != 3:
            raise ValueError("Not a data cube!")
        cube_size = dims[2]
        image_layer_list = range(cube_size)

        # handles the PSFs
        if isinstance(psf, list):
            if len(psf) != cube_size:
                raise ValueError("The number of psf files must match the number of images!")
            psf_file_list = psf
            psf_hdu_list = [0] * cube_size
        else:
            psf_file_list = [psf] * cube_size
            psf_hdu_list = range(cube_size)

        # handles the weight maps
        if isinstance(weight, list):
            if len(weight) != cube_size:
                raise ValueError("The number of weight files must match the number of images!")
            weight_file_list = weight
            weight_hdu_list = [0] * cube_size
            weight_layer_list = [0] * cube_size
        elif weight is None:
            weight_file_list = [None] * cube_size
            weight_hdu_list = [0] * cube_size
            weight_layer_list = [0] * cube_size
        else:
            weight_fits_file = FitsFile(weight)
            if weight_cube_hdu < 0:
                weight_hdu = weight_fits_file.hdu_list[0]
            else:
                weight_hdu = weight_cube_hdu

            weight_dims = weight_fits_file.get_dimensions(weight_hdu)
            if len(weight_dims) == 3:
                # handle weight as data cube
                if dims[2] != cube_size:
                    raise ValueError("The weight map cube doesn't match the image cube")

                weight_file_list = [weight_fits_file] * cube_size
                weight_hdu_list = [weight_hdu] * cube_size
                weight_layer_list = range(cube_size)
            else:
                # weight is a FITS without a datacube, assume MEF
                weight_file_list = [weight_fits_file] * cube_size
                weight_hdu_list = weight_fits_file.hdu_list
                weight_layer_list = [0] * cube_size

        image_list = []
        for psf_file, psf_hdu, weight_file, weight_hdu, image_layer, weight_layer in zip(
                psf_file_list, psf_hdu_list, weight_file_list, weight_hdu_list, image_layer_list,
                weight_layer_list):
            image_list.append(DataCubeSlice(image_cube_file, psf_file, weight_file,
                                            image_hdu=cube_hdu, psf_hdu=psf_hdu,
                                            weight_hdu=weight_hdu,
                                            image_layer=image_layer, weight_layer=weight_layer,
                                            **kwargs))

        for img in image_list:
            self._measurement_images[img.id] = img
        return ImageGroup(images=image_list)

    def print_measurement_images(self, file=sys.stderr):
        """
        Print a human-readable representation of the configured measurement images.

        Parameters
        ----------
        file : file object
            Where to print the representation. Defaults to sys.stderr
        """
        print('Measurement images:', file=file)
        for i in self._measurement_images:
            im = self._measurement_images[i]
            print('Image {}'.format(i), file=file)
            print('      File: {}'.format(im.file), file=file)
            print('       PSF: {}'.format(im.psf_file), file=file)
            print('    Weight: {}'.format(im.weight_file), file=file)

    def add_aperture_photometry(self, target, apertures):
        """
        Flux measurement from the image above the background inside a circular aperture.

        Parameters
        ----------
        target : MeasurementImage object, or leaf MeasurementGroup object with a single image, or a list of either
            Target images on which to measure the aperture photometry. Leaf MeasurementGroup with a single image
            are accepted as a convenience.

        apertures : float, or list of float
            Diameter of the aperture. As different MeasurementImage may not be aligned, nor have equivalent pixel size,
            the aperture is interpreted as diameter in pixels of a circle on the detection image.
            A transformation will be applied for each frame, so the covered area is equivalent.

        Returns
        -------
        list of Aperture objects
            An Aperture object is an internal representation of a property on the measurement frame that contains the
            apertures. To actually get the measurements on the output catalog, you need to add explicitly them to the
            output.

        See Also
        --------
        add_output_column

        Notes
        -----
        This property will generate five columns with the prefix specified by `add_output_column`:
        - ``_flux`` and ``_flux_err``, for the flux and its associated error
        - ``_mag`` and ``_mag_err``, for the magnitude and its associated error
        - ``_flags``, to mark, for instance, saturation, boundary conditions, etc.

        For M apertures and N images, the cells on the output column will be an array of MxN fluxes.

        Examples
        --------
        >>> context = MeasurementConfig()
        >>> measurement_group = MeasurementGroup(load_fits_images(frames, psfs))
        >>> all_apertures = []
        >>> for img in measurement_group:
        >>>     all_apertures.extend(context.add_aperture_photometry(img, [5, 10, 20]))
        >>> context.add_output_column('aperture', all_apertures)
        """
        if not isinstance(target, list):
            target = [target]
        if not isinstance(apertures, list):
            apertures = [apertures]

        apertures = [float(a) for a in apertures]
        outputs = []

        for t in target:
            if isinstance(t, MeasurementGroup):
                if not t.is_leaf():
                    raise Exception('The MeasurementGroup is not a leaf')
                elif len(t) != 1:
                    raise Exception('The MeasurementGroup contains {} images'.format(len(t)))
                t = [i for i in t][0]

            if not isinstance(t, MeasurementImage):
                raise Exception(
                    'Only MeasurementImage supported as targets, got {}'.format(type(t)))
            else:
                if t.id in self._apertures_for_image:
                    raise Exception('Apertures already set for the image {}'.format(t.id))
                self._apertures_for_image[t.id] = cpp.Aperture(apertures)
                outputs.append(self._apertures_for_image[t.id])

        return outputs

    def print_output_columns(self, file=sys.stderr):
        """
        Print a human-readable representation of the configured output columns.

        Parameters
        ----------
        file : file object
            Where to print the representation. Defaults to sys.stderr
        """
        if self._model_fitting_parameter_columns:
            print('Model fitting parameter outputs:', file=file)
            for n, ids in self._model_fitting_parameter_columns:
                print('    {} : {}'.format(n, ids), file=file)
        if self._aperture_columns:
            print('Aperture outputs:', file=file)
            for n, ids in self._aperture_columns:
                print('    {} : {}'.format(n, ids), file=file)

    def add_output_column(self, name, params):
        """
        Add a new set of columns to the output catalog.

        Parameters
        ----------
        name : str
            Name/prefix of the new set of columns
        params : list of columns
            List of properties to add to the output with the given name/prefix. They must be subtype
            of one of the known ones: ParameterBase for model fitting, or Aperture for aperture photometry.

        Raises
        ------
        ValueError
            If the name has already been used
        TypeError
            If any of the parameters are not of a known type (see params)

        See Also
        --------
        aperture.add_aperture_photometry
        model_fitting.ParameterBase
        """
        if name in self._used_names:
            raise ValueError('Column {} is already set'.format(name))
        self._used_names.add(name)

        if not isinstance(params, list):
            params = [params]
        param_type = type(params[0])

        known_subclass = False
        for base in self._column_map:
            if issubclass(param_type, base):
                self._column_map[base].append((name, params))
                known_subclass = True
        if issubclass(param_type, ParameterBase):
            self.model_fitting._register_parameter(params[0])

        if not known_subclass:
            raise TypeError('{} is not a known column type'.format(str(param_type)))


global_measurement_config = MeasurementConfig()
