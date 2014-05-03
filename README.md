mps430-template-library
=======================

This is a library for MSP430 processors implemented as C++ templates. The idea
is based on Rick Kimball's [fabooh](http://fabooh.com/about-fabooh/) and [this
article](http://www.webalice.it/fede.tft/stm32/stm32_gpio_and_template_metaprogramming.html).

C++ templates allow shifting parts of the code from runtime to compile time.
The benefit is reduced RAM usage and the possibility to detect design errors
already when compiling.

The libraries are evolving based mostly on my needs, and cover not all MSP430
variants, nor all functionality. Currently, the MSP430G2x53 is the main
development target.
