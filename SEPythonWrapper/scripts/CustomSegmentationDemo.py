#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright © 2022 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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

import itertools
from argparse import ArgumentParser
from configparser import ConfigParser
from typing import Any, Dict, Tuple

import numpy as np
from astropy.io import fits
from scipy.ndimage import find_objects, label
from sourcextractor import __version__ as seversion
from sourcextractor import pipeline


class CustomSegmentation:
    """
    Naive segmentation algorithm based on a value / error threshold and scipy.ndimage.label
    This is just an example! There is no background subtraction, nor filtering.
    Everything is done in memory, so big images will fill the system memory.

    :param context:
        sourcextractor++ context
    :param detection:
        Detection image
    :param weight:
        Weight image
    :param weight_type:
        Weight type. Only 'weight', 'variance' and 'rms' are supported.
    :param snr:
        Threshold for the value / error check
    """

    @staticmethod
    def __setup_error(weight_img: np.ndarray, weight_type: str):
        """
        Transform the weight image into an error image (sqrt(variance))
        """
        if weight_type == 'weight':
            return np.sqrt(np.reciprocal(weight_img))
        elif weight_type == 'variance':
            return np.sqrt(weight_img)
        elif weight_type == 'rms':
            return weight_type
        raise ValueError(f'Unsupported weight type {weight_type}')

    def __init__(self, context: pipeline.Context, detection: str, weight: str, weight_type: str, snr: float):
        self.__context = context
        self.__count = 0
        self.__detection = fits.open(detection)[0].data
        self.__std = self.__setup_error(fits.open(weight)[0].data, weight_type)
        self.__snr = snr
        self.__next = None

    @property
    def count(self):
        """
        :return: The number of detected sources
        """
        return self.__count

    def set_next_stage(self, next_stage):
        """
        Set the following stage, which must receive sources
        """
        self.__next = next_stage

    @staticmethod
    def __get_pixel_coordinates(labeled: np.ndarray, obj: Tuple[slice, slice], idx: int):
        """
        Given the labeled image, the object bounding box and its detection ID, return the list
        of detected pixels as a tuple (xs, ys)
        """
        cutout = labeled[obj]
        # numpy axes are 0=Y, 1=X!!
        yoff, xoff = np.nonzero(cutout == idx)
        return (xoff + obj[1].start).astype(np.int32), (yoff + obj[0].start).astype(np.int32)

    def __call__(self):
        """
        Apply the threshold, label the image, and find the objects
        """
        # Mask out pixels below the threshold
        mask = (self.__detection / self.__std) < self.__snr
        self.__detection[mask] = 0.
        # Use scipy label
        labeled = label(self.__detection > 0)[0]
        # Use scipy find_object to find the bounding boxes
        detected_objs = find_objects(labeled)
        for idx, obj in enumerate(detected_objs, start=1):
            self.__count += 1
            pixels = self.__get_pixel_coordinates(labeled, obj, idx)
            # We only support a single detection frame in this example
            source = pipeline.Source.create(self.__context, detection_frame=0, detection_id=idx,
                                            pixel_coordinates=pixels)
            self.__next(source)
        self.__next(pipeline.AllFramesDone())


def run_sourcextractor(config: Dict[str, Any], output_path: str):
    """
    Setup the sourcextractor++ pipeline with a custom segmentation stage
    """
    config['output-catalog-filename'] = output_path
    snr = float(config.pop('snr', 5.))
    context = pipeline.Context(config)
    # Generate out custom segmentation
    segmentation = CustomSegmentation(context, config['detection-image'], config['weight-image'], config['weight-type'],
                                      snr)
    with context:
        stages = [segmentation, pipeline.Partition(), pipeline.Grouping(), pipeline.Deblending(),
                  pipeline.Measurement(), pipeline.FitsOutput()]
        pipe = pipeline.Pipeline(stages)
        pipe().get()
    print(f'Done! Found {segmentation.count} sources')


def parse_config_file(path: str) -> Dict[str, Any]:
    """
    Parse a sourcextractor++ (like) config file into a dictionary
    """
    parser = ConfigParser()
    with open(path, 'rt') as fd:
        parser.read_file(itertools.chain(['[main]'], fd))
    return {k: v for k, v in parser.items('main')}


if __name__ == '__main__':
    print(f'Running sourcextractor++ {seversion}')

    parser = ArgumentParser()
    parser.add_argument('--output-file', type=str, metavar='FITS', default='output.fits', help='Output file')
    parser.add_argument('config_file', type=str, metavar='CONFIGURATION', help='Configuration file')

    args = parser.parse_args()
    run_sourcextractor(parse_config_file(args.config_file), args.output_file)
