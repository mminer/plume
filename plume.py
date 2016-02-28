"""Wraps the C library that executes a Lua script."""

import cffi
import msgpack


_C_HEADER = 'scriptrunner.h'
_C_LIBRARY = 'scriptrunner.so'


# Load library:

_ffi = cffi.FFI()

with open(_C_HEADER) as header:
    _ffi.cdef(header.read())

_lib = _ffi.dlopen(_C_LIBRARY)


def run_script(script, table_name='tbl', table=None, instruction_quota=1000):
    """Executes a Lua script, inserting a table into its global scope."""
    if not table:
        table = {}

    table_msgpack = msgpack.packb(table)

    output_msgpack = _lib.run_lua_script(
        script.encode(),
        table_name.encode(),
        table_msgpack,
        instruction_quota,
    )

    output = msgpack.unpackb(_ffi.string(output_msgpack))
    return output
