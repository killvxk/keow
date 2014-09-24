These are the 'kernel' space portions of keow.
They run within a seperate global process and provide all the kernel
services to each keow unix process.

KeowKernel is the main kernel process.

KeowConsole is a handler for console windows. Windows only allows one console per process, so we need an exe to start seperate processes per window.
