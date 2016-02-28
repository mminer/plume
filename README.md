# Plume

Python + Lua + MessagePack

A small C library that runs a Lua script, callable from Python. The Python
function accepts a dictionary, which is encoded to MessagePack, which is passed
to the C function, which sets it as a global table in the Lua environment.
