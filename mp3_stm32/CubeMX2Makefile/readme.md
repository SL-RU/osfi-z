#STM32CubeMX to Makefile

IT IS REQUIRED FOR: https://github.com/SL-RU/stm32-emacs

This program generates a Makefile from STM32CubeMX (http://www.st.com/stm32cube) created project. It is intended to be used along with GNU Make utility (www.gnu.org/software/make) and GNU tools for ARM (https://launchpad.net/gcc-arm-embedded) to compile STM32 firmware. Refer to my blog post http://www.ba0sh1.com/opensource-stm32-development for setup of integrated development environment.  

Also it generates project.el for Emacs Ede project support. It contains ede-cpp-root-project which allows to use autocompletion and other Cedet features.

Copyright (c) 2016, Baoshi Zhu. All rights reserved.
Modified by Alexander Lutsai, 2016

Source code in this project is governed by Apache License 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
