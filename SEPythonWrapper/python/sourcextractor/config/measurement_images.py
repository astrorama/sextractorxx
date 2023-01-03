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

import os
import re
import sys

import _SEPythonConfig as cpp

if sys.version_info.major < 3:
    from StringIO import StringIO
else:
    from io import StringIO


class FitsFile(cpp.FitsFile):
    def __init__(self, filename):
        super(FitsFile, self).__init__(str(filename))
        self.hdu_list = [i for i in self.image_hdus]

    def __iter__(self):
        return iter(self.hdu_list)

    def get_headers(self, hdu):
        d = {}
        headers = super(FitsFile, self).get_headers(hdu)

        try:
            it = iter(headers)
            while it:
                a = next(it)
                d[a.key()] = headers[a.key()]
        except StopIteration:
            pass

        return d


class MeasurementImage(cpp.MeasurementImage):
    """
    A MeasurementImage is the processing unit for SourceXtractor++. Measurements and model fitting can be done
    over one, or many, of them. It models the image, plus its associated weight file, PSF, etc.

    Parameters
    ----------
    fits_file : str or FitsFile object
        The path to a FITS image, or an instance of FitsFile
    psf_file : str
        The path to a PSF. It can be either a FITS image, or a PSFEx model.
    weight_file : str or FitsFile
        The path to a FITS image with the pixel weights, or an instance of FitsFile
    gain : float
        Image gain. If None, `gain_keyword` will be used instead.
    gain_keyword : str
        Keyword for the header containing the gain.
    saturation : float
        Saturation value. If None, `saturation_keyword` will be used instead.
    saturation_keyword : str
        Keyword for the header containing the saturation value.
    flux_scale : float
        Flux scaling. Each pixel value will be multiplied by this. If None, `flux_scale_keyword` will be used
        instead.
    flux_scale_keyword : str
        Keyword for the header containing the flux scaling.
    weight_type : str
        The type of the weight image. It must be one of:

            - none
                The image itself is used to compute internally a constant variance (default)
            - background
                The image itself is used to compute internally a variance map
            - rms
                The weight image must contain a weight-map in units of absolute standard deviations
                (in ADUs per pixel).
            - variance
                The weight image must contain a weight-map in units of relative variance.
            - weight
                The weight image must contain a weight-map in units of relative weights. The data are converted
                to variance units.
    weight_absolute : bool
        If False, the weight map will be scaled according to an absolute variance map built from the image itself.
    weight_scaling : float
        Apply an scaling to the weight map.
    weight_threshold : float
        Pixels with weights beyond this value are treated just like pixels discarded by the masking process.
    constant_background : float
        If set a constant background of that value is assumed for the image instead of using automatic detection
    image_hdu : int
        For multi-extension FITS file specifies the HDU number for the image. Default 0 (primary HDU)
    psf_hdu : int
        For multi-extension FITS file specifies the HDU number for the psf. Defaults to the same value as image_hdu
    weight_hdu : int
        For multi-extension FITS file specifies the HDU number for the weight. Defaults to the same value as image_hdu
    """

    def _set_checked(self, attr_name, value):
        try:
            setattr(self, attr_name, value)
        except Exception:
            expected_type = type(getattr(self, attr_name))
            raise TypeError('Expecting {} for {}, got {}'.format(expected_type.__name__, attr_name,
                                                                 type(value).__name__))

    def __init__(self, fits_file, psf_file=None, weight_file=None, gain=None,
                 gain_keyword='GAIN', saturation=None, saturation_keyword='SATURATE',
                 flux_scale=None, flux_scale_keyword='FLXSCALE',
                 weight_type='none', weight_absolute=False, weight_scaling=1.,
                 weight_threshold=None, constant_background=None,
                 image_hdu=0, psf_hdu=None, weight_hdu=None
                 ):
        """
        Constructor.
        """
        if isinstance(fits_file, FitsFile):
            hdu_list = fits_file
            file_path = fits_file.filename
        else:
            hdu_list = FitsFile(fits_file)
            file_path = fits_file

        if isinstance(weight_file, FitsFile):
            weight_file = weight_file.filename

        super(MeasurementImage, self).__init__(os.path.abspath(file_path),
                                               os.path.abspath(psf_file) if psf_file else '',
                                               os.path.abspath(weight_file) if weight_file else '')

        if image_hdu < 0 or (weight_hdu is not None and weight_hdu < 0) or (
                psf_hdu is not None and psf_hdu < 0):
            raise ValueError('HDU indices start at 0')

        self.meta = {
            'IMAGE_FILENAME': self.file,
            'PSF_FILENAME': self.psf_file,
            'WEIGHT_FILENAME': self.weight_file
        }

        self.meta.update(hdu_list.get_headers(image_hdu))

        if gain is not None:
            self._set_checked('gain', gain)
        elif gain_keyword in self.meta:
            self.gain = float(self.meta[gain_keyword])
        else:
            self.gain = 0.

        if saturation is not None:
            self._set_checked('saturation', saturation)
        elif saturation_keyword in self.meta:
            self.saturation = float(self.meta[saturation_keyword])
        else:
            self.saturation = 0.

        if flux_scale is not None:
            self._set_checked('flux_scale', flux_scale)
        elif flux_scale_keyword in self.meta:
            self.flux_scale = float(self.meta[flux_scale_keyword])
        else:
            self.flux_scale = 1.

        self._set_checked('weight_type', weight_type)
        self._set_checked('weight_absolute', weight_absolute)
        self._set_checked('weight_scaling', weight_scaling)
        if weight_threshold is None:
            self.has_weight_threshold = False
        else:
            self.has_weight_threshold = True
            self._set_checked('weight_threshold', weight_threshold)

        if constant_background is not None:
            self.is_background_constant = True
            self._set_checked('constant_background_value', constant_background)
        else:
            self.is_background_constant = False
            self.constant_background_value = -1

        self._set_checked('image_hdu', image_hdu)

        if psf_hdu is None:
            self._set_checked('psf_hdu', image_hdu)
        else:
            self._set_checked('psf_hdu', psf_hdu)

        if weight_hdu is None:
            self._set_checked('weight_hdu', image_hdu)
        else:
            self._set_checked('weight_hdu', weight_hdu)

    def __str__(self):
        """
        Returns
        -------
        str
            Human readable representation for the object
        """
        return 'Image {}: {} / {}, PSF: {} / {}, Weight: {} / {}'.format(
            self.id, self.meta['IMAGE_FILENAME'], self.image_hdu, self.meta['PSF_FILENAME'],
            self.psf_hdu,
            self.meta['WEIGHT_FILENAME'], self.weight_hdu)


class DataCubeSlice(MeasurementImage):
    def __init__(self, fits_file, psf_file=None, weight_file=None, gain=None,
                 gain_keyword='GAIN', saturation=None, saturation_keyword='SATURATE',
                 flux_scale=None, flux_scale_keyword='FLXSCALE',
                 weight_type='none', weight_absolute=False, weight_scaling=1.,
                 weight_threshold=None, constant_background=None,
                 image_hdu=0, psf_hdu=None, weight_hdu=None,
                 image_layer=0, weight_layer=0):
        super(DataCubeSlice, self).__init__(fits_file, psf_file, weight_file, gain,
                                            gain_keyword, saturation, saturation_keyword,
                                            flux_scale, flux_scale_keyword,
                                            weight_type, weight_absolute, weight_scaling,
                                            weight_threshold, constant_background,
                                            image_hdu, psf_hdu, weight_hdu)

        self.is_data_cube = True
        self.image_layer = image_layer
        self.weight_layer = weight_layer

    def __str__(self):
        """
        Returns
        -------
        str
            Human readable representation for the object
        """
        return 'DataCubeSlice {}: {} / {} / {}, PSF: {} / {}, Weight: {} / {} / {}'.format(
            self.id, self.meta['IMAGE_FILENAME'], self.image_hdu, self.image_layer,
            self.meta['PSF_FILENAME'], self.psf_hdu,
            self.meta['WEIGHT_FILENAME'], self.weight_hdu, self.weight_layer)


class ImageGroup(object):
    """
    Models the grouping of images. Measurement can *not* be made directly on instances of this type.
    The configuration must be "frozen" before creating a MeasurementGroup

    See Also
    --------
    MeasurementGroup
    """

    def __init__(self, **kwargs):
        """
        Constructor. It is not recommended to be used directly. Use instead load_fits_image or load_fits_images.
        """
        self.__images = []
        self.__subgroups = None
        self.__subgroup_names = set()
        if len(kwargs) != 1 or ('images' not in kwargs and 'subgroups' not in kwargs):
            raise ValueError('ImageGroup only takes as parameter one of "images" or "subgroups"')
        key = list(kwargs.keys())[0]
        if key == 'images':
            if isinstance(kwargs[key], list):
                self.__images = kwargs[key]
            else:
                self.__images = [kwargs[key]]
        if key == 'subgroups':
            self.__subgroups = kwargs[key]
            for name, _ in self.__subgroups:
                self.__subgroup_names.add(name)

    def __len__(self):
        """
        See Also
        --------
        is_leaf

        Returns
        -------
        int
            How may subgroups or images are there in this group
        """
        if self.__subgroups:
            return len(self.__subgroups)
        else:
            return len(self.__images)

    def __iter__(self):
        """
        Allows to iterate on the contained subgroups or images

        See Also
        --------
        is_leaf

        Returns
        -------
        iterator
        """
        if self.__subgroups:
            return self.__subgroups.__iter__()
        else:
            return self.__images.__iter__()

    def split(self, grouping_method):
        """
        Splits the group in various subgroups, applying a filter on the contained images. If the group has
        already been split, applies the split to each subgroup.

        Parameters
        ----------
        grouping_method : callable
            A callable that receives as a parameter the list of contained images, and returns
            a list of tuples, with the grouping key value, and the list of grouped images belonging to the given key.

        See Also
        --------
        ByKeyword
        ByPattern

        Raises
        -------
        ValueError
            If some images have not been grouped by the callable.
        """
        if self.__subgroups:
            # if we are already subgrouped, apply the split to the subgroups
            for _, sub_group in self.__subgroups:
                sub_group.split(grouping_method)
        else:
            subgrouped_images = grouping_method(self.__images)
            if sum(len(p[1]) for p in subgrouped_images) != len(self.__images):
                self.__subgroups = None
                raise ValueError('Some images were not grouped')
            self.__subgroups = []
            for k, im_list in subgrouped_images:
                assert k not in self.__subgroup_names
                self.__subgroup_names.add(k)
                self.__subgroups.append((k, ImageGroup(images=im_list)))
            self.__images = []

    def add_images(self, images):
        """
        Add new images to the group.

        Parameters
        ----------
        images : list of, or a single, MeasurementImage

        Raises
        ------
        ValueError
            If the group has been split, no new images can be added.
        """
        if self.__subgroups is not None:
            raise ValueError('ImageGroup is already subgrouped')
        if isinstance(images, MeasurementImage):
            self.__images.append(images)
        else:
            self.__images.extend(images)

    def add_subgroup(self, name, group):
        """
        Add a subgroup to a group.

        Parameters
        ----------
        name : str
            The new of the new group

        group : ImageGroup
        """
        if self.__subgroups is None:
            raise Exception('ImageGroup is not subgrouped yet')
        if name in self.__subgroup_names:
            raise Exception('Subgroup {} alread exists'.format(name))
        self.__subgroup_names.add(name)
        self.__subgroups.append((name, group))

    def is_leaf(self):
        """
        Returns
        -------
        bool
            True if the group is a leaf group
        """
        return self.__subgroups is None

    def __getitem__(self, name):
        """
        Get a subgroup.

        Parameters
        ----------
        name : str
            The name of the subgroup.

        Returns
        -------
        ImageGroup
            The matching group.

        Raises
        ------
        ValueError
            If the group has not been split.
        KeyError
            If the group has not been found.
        """
        if self.__subgroups is None:
            raise ValueError('ImageGroup is not subgrouped yet')
        try:
            return next(x for x in self.__subgroups if x[0] == name)[1]
        except StopIteration:
            raise KeyError('Group {} not found'.format(name))

    def print(self, prefix='', show_images=False, file=sys.stderr):
        """
        Print a human-readable representation of the group.

        Parameters
        ----------
        prefix : str
            Print each line with this prefix. Used internally for indentation.
        show_images : bool
            Show the images belonging to a leaf group.
        file : file object
            Where to print the representation. Defaults to sys.stderr
        """
        if self.__subgroups is None:
            print('{}Image List ({})'.format(prefix, len(self.__images)), file=file)
            if show_images:
                for im in self.__images:
                    print('{}  {}'.format(prefix, im), file=file)
        else:
            print('{}Image sub-groups: {}'.format(prefix,
                                                  ','.join(str(x) for x, _ in self.__subgroups)),
                  file=file)
            for name, group in self.__subgroups:
                print('{}  {}:'.format(prefix, name), file=file)
                group.print(prefix + '    ', show_images, file)

    def __str__(self):
        """
        Returns
        -------
        str
            A human-readable representation of the group
        """
        string = StringIO()
        self.print(show_images=True, file=string)
        return string.getvalue()


class ByKeyword(object):
    """
    Callable that can be used to split an ImageGroup by a keyword value (i.e. FILTER).

    Parameters
    ----------
    key : str
        FITS header keyword (i.e. FILTER)

    See Also
    --------
    ImageGroup.split
    """

    def __init__(self, key):
        """
        Constructor.
        """
        self.__key = key

    def __call__(self, images):
        """
        Parameters
        ----------
        images : list of MeasurementImage
            List of images to group

        Returns
        -------
        list of tuples of str and list of MeasurementImage
            i.e. [
                (R, [frame_r_01.fits,  frame_r_02.fits]),
                (G, [frame_g_01.fits,  frame_g_02.fits])
            ]
        """
        result = {}
        for im in images:
            if self.__key not in im.meta:
                raise KeyError('The image {}[{}] does not contain the key {}'.format(
                    im.meta['IMAGE_FILENAME'], im.image_hdu, self.__key
                ))
            if im.meta[self.__key] not in result:
                result[im.meta[self.__key]] = []
            result[im.meta[self.__key]].append(im)
        return [(k, result[k]) for k in result]


class ByPattern(object):
    """
    Callable that can be used to split an ImageGroup by a keyword value (i.e. FILTER), applying a regular
    expression and using the first matching group as key.

    Parameters
    ----------
    key : str
        FITS header keyword
    pattern : str
        Regular expression. The first matching group will be used as grouping key.

    See Also
    --------
    ImageGroup.split
    """

    def __init__(self, key, pattern):
        """
        Constructor.
        """
        self.__key = key
        self.__pattern = pattern

    def __call__(self, images):
        """
        Parameters
        ----------
        images : list of MeasurementImage
            List of images to group

        Returns
        -------
        list of tuples of str and list of MeasurementImage
        """
        result = {}
        for im in images:
            if self.__key not in im.meta:
                raise KeyError('The image {}[{}] does not contain the key {}'.format(
                    im.meta['IMAGE_FILENAME'], im.image_hdu, self.__key
                ))
            group = re.match(self.__pattern, im.meta[self.__key]).group(1)
            if group not in result:
                result[group] = []
            result[group].append(im)
        return [(k, result[k]) for k in result]


class MeasurementGroup(object):
    """
    Once an instance of this class is created from an ImageGroup, its configuration is "frozen". i.e.
    no new images can be added, or no new grouping applied.

    Parameters
    ----------
    image_group : ImageGroup
    """

    def __init__(self, image_group, is_subgroup=False):
        """
        Constructor.
        """
        self.__images = None
        self.__subgroups = None
        if image_group.is_leaf():
            self.__images = [im for im in image_group]
        else:
            self.__subgroups = [(n, MeasurementGroup(g, is_subgroup=True)) for n, g in image_group]

    def __iter__(self):
        """
        Returns
        -------
        iterator
        """
        if self.__subgroups:
            return self.__subgroups.__iter__()
        else:
            return self.__images.__iter__()

    def __getitem__(self, index):
        """
        The subgroup with the given name or image with the given index depending on whether this is a leaf group.

        Parameters
        ----------
        index : str or int
            Subgroup name or image index

        Returns
        -------
        MeasurementGroup or MeasurementImage

        Raises
        ------
        KeyError
            If we can't find what we want
        """

        if self.__subgroups:
            try:
                return next(x for x in self.__subgroups if x[0] == index)[1]
            except StopIteration:
                raise KeyError('Group {} not found'.format(index))
        else:
            try:
                return self.__images[index]
            except:
                raise KeyError('Image #{} not found'.format(index))

    def __len__(self):
        """
        Returns
        -------
        int
            Number of subgroups, or images contained within the group
        """
        if self.__subgroups:
            return len(self.__subgroups)
        else:
            return len(self.__images)

    def is_leaf(self):
        """
        Returns
        -------
        bool
            True if the group is a leaf group
        """
        return self.__subgroups is None

    def print(self, prefix='', show_images=False, file=sys.stderr):
        """
        Print a human-readable representation of the group.

        Parameters
        ----------
        prefix : str
            Print each line with this prefix. Used internally for indentation.
        show_images : bool
            Show the images belonging to a leaf group.
        file : file object
            Where to print the representation. Defaults to sys.stderr
        """
        if self.__images:
            print('{}Image List ({})'.format(prefix, len(self.__images)), file=file)
            if show_images:
                for im in self.__images:
                    print('{}  {}'.format(prefix, im), file=file)
        if self.__subgroups:
            print('{}Measurement sub-groups: {}'.format(prefix, ','.join(
                x for x, _ in self.__subgroups)), file=file)
            for name, group in self.__subgroups:
                print('{}  {}:'.format(prefix, name), file=file)
                group.print(prefix + '    ', show_images, file=file)

    def __str__(self):
        """
        Returns
        -------
        str
            A human-readable representation of the group
        """
        string = StringIO()
        self.print(show_images=True, file=string)
        return string.getvalue()
