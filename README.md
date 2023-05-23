# Unix Domain Socket Test
## Overview
An example builds a simple server and client systems that transfer data via unix domain socket.
The example builds on the linux kernel. There are client application, server application, client kernel module, server kernel module and they will transfer data together.
## Install
Run the makefile in Sock_UDP then it generate a bin directory. It contains client application, server application, client kernel module, server kernel module.
- make
## Use
### Kernel Module
Show the parameters that need to run.
- modinfo <name_of_kernel_module>
