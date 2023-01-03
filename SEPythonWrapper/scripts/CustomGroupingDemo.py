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
import logging
import sys
from argparse import ArgumentParser
from configparser import ConfigParser
from typing import Any, Dict, List, Union

import numpy as np
from sklearn.cluster import AgglomerativeClustering
from sklearn.neighbors import radius_neighbors_graph
from sourcextractor import __version__ as seversion
from sourcextractor import pipeline

logger = logging.getLogger(__name__)


class CustomGrouping:
    """
    Grouping based on AgglomerativeClustering. This is just an example of how to perform custom grouping
    on sourcextractor++.

    :param context:
        sourcextractor++ context
    :param kron_scale:
        Multiply the Kron radius by this factor to compute the bounding sphere
    :param distance_threshold:
        Distance threshold for AgglomerativeClustering. 'auto' uses twice the mean radius as an approximation.
    """

    def __init__(self, context: pipeline.Context, kron_scale: float = 2.5,
                 distance_threshold: Union[float, str] = 'auto'):
        self.__context = context
        self.__kron_scale = kron_scale
        self.__distance_threshold = distance_threshold
        self.__to_group = []

    def set_next_stage(self, next_stage):
        """
        Set the following stage, which must receive groups
        """
        self.__next = next_stage

    def __filter_sources(self, event: pipeline.ProcessSourcesEvent):
        """
        Return a list of sources to flush, and sources to keep
        """
        flush, no_flush = [], []
        for source in self.__to_group:
            if event.must_process(source):
                flush.append(source)
            else:
                no_flush.append(source)
        return flush, no_flush

    def __group(self, sources: List[pipeline.Source]):
        """
        Group the given list of sources via AgglomerativeClustering
        :return:
            A list of Groups
        """
        if not sources:
            return []

        logger.info('Grouping %d sources', len(sources))

        coordinates = np.asarray([(source.pixel_centroid_x, source.pixel_centroid_y) for source in sources])
        radius = 2. * np.asarray([source.kron_radius * self.__kron_scale for source in sources])

        if self.__distance_threshold == 'auto':
            threshold = np.mean(radius)
        else:
            threshold = self.__distance_threshold

        A = radius_neighbors_graph(coordinates, radius=radius, mode='connectivity', include_self=True)
        clustering = AgglomerativeClustering(connectivity=A, n_clusters=None, distance_threshold=threshold)
        clusters = clustering.fit_predict(coordinates)

        logger.info('Found %d groups', clusters.max())

        groups = [[] for _ in range(len(clusters))]
        for i, cluster_id in enumerate(clusters):
            groups[cluster_id].append(sources[i])

        groups = list(map(lambda group: pipeline.Group.create(self.__context, group), groups))
        return groups

    def __flush(self, event: pipeline.ProcessSourcesEvent):
        """
        Group and flush the sources kept in memory
        """
        flusheable, self.__to_group = self.__filter_sources(event)
        groups = self.__group(flusheable)
        for group in groups:
            self.__next(group)
        # We need to forward the event
        self.__next(event)

    def __call__(self, obj):
        """
        Called by segmentation. Use ProcessSourceEvent.must_source to see if stored sources must be
        grouped and flushed.
        :param obj:
            Either a ProcessSourceEvent sent by Segmentation to trigger a flush, or a
            segmented source
        """
        if isinstance(obj, pipeline.ProcessSourcesEvent):
            self.__flush(obj)
        else:
            self.__to_group.append(obj)


def run_sourcextractor(config: Dict[str, Any], output_path: str):
    """
    Setup the sourcextractor++ pipeline with a custom grouping stage
    """
    config['output-catalog-filename'] = output_path
    kron_scale = float(config.get('auto-kron-factor', 2.5))
    distance_threshold = config.pop('grouping-distance-threshold', 'auto')
    if distance_threshold != 'auto':
        distance_threshold = float(distance_threshold)
    context = pipeline.Context(config)
    # Generate out custom grouping
    grouping = CustomGrouping(context, kron_scale, distance_threshold)
    with context:
        stages = [pipeline.Segmentation(), pipeline.Partition(), grouping, pipeline.Deblending(),
                  pipeline.Measurement(), pipeline.FitsOutput()]
        pipe = pipeline.Pipeline(stages)
        pipe().get()
    print(f'Done!')


def parse_config_file(path: str) -> Dict[str, Any]:
    """
    Parse a sourcextractor++ (like) config file into a dictionary
    """
    parser = ConfigParser()
    with open(path, 'rt') as fd:
        parser.read_file(itertools.chain(['[main]'], fd))
    return {k: v for k, v in parser.items('main')}


if __name__ == '__main__':
    err_handler = logging.StreamHandler(sys.stderr)
    err_handler.setLevel(logging.INFO)
    logger.addHandler(err_handler)
    logger.setLevel(logging.INFO)

    print(f'Running sourcextractor++ {seversion}')

    parser = ArgumentParser()
    parser.add_argument('--output-file', type=str, metavar='FITS', default='output.fits', help='Output file')
    parser.add_argument('config_file', type=str, metavar='CONFIGURATION', help='Configuration file')

    args = parser.parse_args()
    run_sourcextractor(parse_config_file(args.config_file), args.output_file)
