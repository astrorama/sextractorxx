# -*- coding: utf-8 -*-
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

from _SEPythonModule import *


class Pipeline:
    """
    Wrap a set of pipeline stages and chains them

    :param stages:
        List of pipeline stages
    """

    def __init__(self, stages):
        if len(stages) < 1:
            raise ValueError('Expecting at least one stage')
        self.__first = stages[0]
        self.__last = stages[-1]
        for a, b in zip(stages[:-1], stages[1:]):
            a.set_next_stage(b)

    def __call__(self):
        """
        Trigger the execution of the pipeline
        :return:
            The last chain of the pipeline
        """
        self.__first()
        return self.__last


class DefaultPipeline(Pipeline):
    """
    Default implementation of the sourcextractor++ pipeline, equivalent to running
    the CLI manually
    """

    def __init__(self):
        super().__init__([Segmentation(), Partition(), Grouping(), Deblending(), Measurement(), FitsOutput()])
