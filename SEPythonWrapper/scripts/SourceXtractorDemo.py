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
import os.path
from argparse import ArgumentParser
from configparser import ConfigParser
from datetime import timedelta
from typing import Any, Dict

import h5py
import numpy as np
from sourcextractor import __version__ as seversion
from sourcextractor import pipeline


class SNRFilter:
    """
    Drop sources with a signal-to-noise below the configured limit.
    It expect sources, so it must be inserted into the pipeline *before* the partitioning

    :param snr: float
        Signal-to-noise ratio cut
    """

    def __init__(self, snr: float):
        self.__snr = snr
        self.__next = None
        self.__dropped = []

    @property
    def dropped(self):
        return self.__dropped

    def set_next_stage(self, stage):
        self.__next = stage

    def __call__(self, obj):
        """
        Apply the SNR filter
        """
        if isinstance(obj, pipeline.Source):
            if obj.isophotal_flux / obj.isophotal_flux_err < self.__snr:
                self.__dropped.append(obj)
                return
        self.__next(obj)


class StoreStamps:
    """
    Store the detection stamps into the HDF5 file

    :param hd5: h5py.File
        Output HDF5 file
    """

    def __init__(self, hd5: h5py.File):
        self.__hd5 = hd5
        self.__next = None

    def set_next_stage(self, stage):
        self.__next = stage

    def __store_stamp(self, source):
        stamp = source.detection_filtered_stamp
        dataset = self.__hd5.create_dataset(f'sources/{source.source_id}', data=stamp)
        dataset.attrs.create('CLASS', 'IMAGE', dtype='S6')
        dataset.attrs.create('IMAGE_VERSION', '1.2', dtype='S4')
        dataset.attrs.create('IMAGE_SUBCLASS', 'IMAGE_GRAYSCALE', dtype='S16')
        dataset.attrs.create('IMAGE_MINMAXRANGE', [np.min(stamp), np.max(stamp)])

    def __call__(self, obj):
        """
        Supports being called with a single Source, or with a Group of sources
        """
        if isinstance(obj, pipeline.Source):
            self.__store_stamp(obj)
        elif isinstance(obj, pipeline.Group):
            [self.__store_stamp(source) for source in obj]
        elif not isinstance(obj, pipeline.ProcessSourcesEvent):
            print(f'Unknown {type(obj)}')
        self.__next(obj)


def run_sourcextractor(config: Dict[str, Any], output_path: str, stamps: bool):
    """
    Setup the sourcextractor++ pipeline, run it, and write the output to an HDF5 file
    """
    h5 = None
    if output_path.endswith('.h5'):
        h5 = h5py.File(output_path, 'w')
        Output = pipeline.NumpyOutput
    else:
        Output = pipeline.FitsOutput
        config['output-catalog-filename'] = output_path

    timeout = config.pop('timeout', timedelta(days=365))
    snr_filter = SNRFilter(float(config.pop('snr', 5)))
    with pipeline.Context(config):
        stages = [pipeline.Segmentation(), pipeline.Partition(), snr_filter, pipeline.Grouping(), pipeline.Deblending()]
        if stamps and h5 is not None:
            stages.append(StoreStamps(h5))
        stages.extend([pipeline.Measurement(), Output()])
        pipe = pipeline.Pipeline(stages)
        result = pipe().get(timeout=timeout)
        print(f'Dropped {len(snr_filter.dropped)} sources')

    if h5 is not None:
        h5.create_dataset(os.path.basename(config['detection-image']), data=result)
        h5.close()
    print(f'{output_path} created!')


def parse_config_file(path: str) -> Dict[str, Any]:
    """
    Parse a sourcextractor++ (like) config file into a dictionary
    """
    parser = ConfigParser()
    with open(path, 'rt') as fd:
        parser.read_file(itertools.chain(['[main]'], fd))
    return {k: v for k, v in parser.items('main')}


# Entry point
if __name__ == '__main__':
    print(f'Running sourcextractor++ {seversion}')

    parser = ArgumentParser()
    parser.add_argument('--output-file', type=str, metavar='HDF5', default='output.h5', help='Output file')
    parser.add_argument('--with-stamps', action='store_true', default=False, help='Store source stamps')
    parser.add_argument('config_file', type=str, metavar='CONFIGURATION', help='Configuration file')

    args = parser.parse_args()
    run_sourcextractor(parse_config_file(args.config_file), args.output_file, args.with_stamps)
