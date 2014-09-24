These are the 'user' space portions of keow.
They run within each keow unix process to handle interaction
with the keow kernel.

The stub is a very small win32 executable used to get windows to start
a new process. It is very small to ensure that it minimises the chance
of it's memory space overlapping a memory area that the unix code needs.

The kernel loads the stub, then allocates the unix memory needed
and then loads the user dll which has the real functionality.
This means that the memory intensive parts of the keow user portion
will load into memory that doesn't interfer with the unix code.
