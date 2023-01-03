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
import threading
from argparse import ArgumentParser
from configparser import ConfigParser
from typing import Any, Dict

import matplotlib.pyplot as plt
from astropy.io import fits
from matplotlib.animation import FuncAnimation
from matplotlib.colors import SymLogNorm
from sourcextractor import __version__ as seversion
from sourcextractor import pipeline


class LivePreview:
    """
    Display sources on a matplotlib figure as they are being detected
    """

    def __init__(self, image):
        self.__next_stage = None
        self.__figure, self.__ax = plt.subplots()
        self.__height, self.__width = fits.open(image)[0].data.shape
        self.__ani = FuncAnimation(self.__figure, self.__update, interval=1000, init_func=self.__init_fig, blit=False)
        self.__artists = []
        self.__norm = SymLogNorm(10)

    def set_next_stage(self, next_stage):
        self.__next_stage = next_stage

    def __init_fig(self):
        self.__ax.set_xlim(0, self.__width)
        self.__ax.set_ylim(0, self.__height)
        return []

    def __update(self, frame: int):
        artists = self.__artists
        self.__artists = []
        return artists

    def __add_stamp(self, source):
        stamp = source.detection_filtered_stamp
        pos_x, pos_y = source.pixel_centroid_x, source.pixel_centroid_y
        left, right = pos_x - stamp.shape[1] // 2, pos_x + stamp.shape[1] // 2
        bottom, top = pos_y - stamp.shape[0] // 2, pos_y + stamp.shape[1] // 2
        self.__artists.append(
            self.__ax.imshow(stamp, origin='lower', extent=(left, right, bottom, top), cmap='Greys_r', norm=self.__norm)
        )

    def show(self):
        self.__figure.show()

    def stop(self):
        self.__ani.event_source.stop()

    def __call__(self, obj):
        if isinstance(obj, pipeline.Source):
            self.__add_stamp(obj)
        elif isinstance(obj, pipeline.Group):
            [self.__add_stamp(source) for source in obj]
        self.__next_stage(obj)


def run_sourcextractor(config: Dict[str, Any], output_path: str):
    """
    Setup the sourcextractor++ pipeline
    """
    config['output-catalog-filename'] = output_path
    live = LivePreview(config['detection-image'])
    with pipeline.Context(config):
        stages = [pipeline.Segmentation(), pipeline.Partition(),
                  pipeline.Grouping(), pipeline.Deblending(), live, pipeline.Measurement(), pipeline.FitsOutput()]
        pipe = pipeline.Pipeline(stages)

    # UI must run on the main thread, so sourcextractor++ must run on another
    sourcex_thread = threading.Thread(target=lambda: pipe().get())
    sourcex_thread.start()
    while sourcex_thread.is_alive():
        live.show()
        plt.pause(0.05)
    sourcex_thread.join()
    print(f'Done!')
    live.stop()
    plt.show(block=True)


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
