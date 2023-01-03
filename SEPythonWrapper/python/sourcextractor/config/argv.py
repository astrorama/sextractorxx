# -*- coding: utf-8 -*-

# Copyright © 2019 Université de Genève, LMU Munich - Faculty of Physics, IAP-CNRS/Sorbonne Université
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
from glob import glob


class Arguments(object):
    """
    This helper class automatically parses the arguments received by the Python script via
    sys.argv, using the set of key-value parameters received on its constructor to cast the types
    and define the defaults.

    Parameters
    ----------
    kwargs
        A set of key-values with the defaults, or the types of the expected values. For instance, it is valid to do
        both:

        key1 = 32.
        key2 = float

        For the first case - a value -, if no value is passed via sys.argv, this will be the default. Otherwise,
        its type (float) will be used to cast the received value.

        For the second case - a type -, if no value is passed via sys.argv, the parameter will default to None.
        Otherwise, the type will be used to cast the received value.

        Callables are also accepted. They should accept to be called with no parameters, returning the default,
        or with a single string, returning the parsed value.

        Values can be accessed later as attributes:

        instance.key1 and instance.key2

    Raises
    ------
    ValueError
        If the cast of the parameter failed (i.e. an invalid integer or float format)
    KeyError
        If additional unknown parameters are received
    """

    def __init__(self, **kwargs):
        """
        Constructor.
        """
        argv = dict([arg.split('=', 1) if '=' in arg else (arg, True) for arg in sys.argv[1:]])

        self.__args = dict()
        for arg_name, arg_spec in kwargs.items():
            if isinstance(arg_spec, type):
                arg_type = arg_spec
                arg_default = None
            elif callable(arg_spec):
                arg_type = arg_spec
                arg_default = arg_spec()
            else:
                arg_type = type(arg_spec)
                arg_default = arg_spec

            if arg_name in argv:
                try:
                    self.__args[arg_name] = arg_type(argv[arg_name])
                except ValueError:
                    raise ValueError('Invalid value for {}: {}'.format(arg_name, argv[arg_name]))
                argv.pop(arg_name)
            else:
                self.__args[arg_name] = arg_default

        if len(argv) != 0:
            raise KeyError('Unknown parameters found: {}'.format(', '.join(argv.keys())))

    def __getattr__(self, item):
        """

        Parameters
        ----------
        item : str
            Name of the parameter to retrieve

        Returns
        -------
            The parsed value of the parameter

        Raises
        ------
        AttributeError
            If item has not been defined on construction
        """
        if item not in self.__args:
            raise AttributeError(item)
        return self.__args[item]

    def __str__(self):
        """
        Returns
        -------
        str
            Human readable representation for the object
        """
        return str(self.__args)

    def __repr__(self):
        """
        Returns
        -------
        str
            String representation for the object
        """
        return repr(self.__args)


class FileList(object):
    """
    Helper class for receiving a globing pattern as a parameter, defining a list of files
    - i.e. measurement images or PSFs.
    It is an iterable, and can be passed directly to load_fits_images.

    Parameters
    ----------
    value : str
        A file globing expression. i.e "*.psf", "band_[r|i|g]_*.fits" or similar.
        The result is always stored and returned in alphabetical order, so the order between two
        file lists - i.e frame image and PSF - is consistent and a matching can be done easily between them.

    See Also
    --------
        glob.glob : Return a list of paths matching a pathname pattern.
    """

    def __init__(self, value):
        """
        Constructor.
        """
        self.__files = sorted(glob(value))

    def __iter__(self):
        """
        Returns
        -------
            iterator
        """
        return iter(self.__files)

    def __len__(self):
        """
        Returns
        -------
        int
            Length of the list
        """
        return len(self.__files)

    def __str__(self):
        """
        Returns
        -------
        str
            Human readable representation for the object
        """
        return str(self.__files)

    def __repr__(self):
        """
        Returns
        -------
        str
            String representation for the object
        """
        return repr(self.__files)
