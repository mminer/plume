"""Sets up the shared C library."""

import cffi
import os


_C_HEADER = 'scriptrunner.h'
_C_LIBRARY = 'scriptrunner.so'


def load_lib():
    """Loads the foreign function interface and shared library."""
    lib_directory = os.path.dirname(__file__)
    header_path = os.path.join(lib_directory, _C_HEADER)
    ffi = cffi.FFI()

    with open(header_path) as header:
        ffi.cdef(header.read())

    lib_path = os.path.join(lib_directory, _C_LIBRARY)
    lib = ffi.dlopen(lib_path)
    return ffi, lib
