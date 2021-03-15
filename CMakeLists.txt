# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE src/main.c)
target_sources_ifdef(CONFIG_ST25R3911B_LIB app PRIVATE st25r3911b/st25r3911b_spi.c)
target_sources_ifdef(CONFIG_ST25R3911B_LIB app PRIVATE st25r3911b/st25r3911b_common.c)
target_sources_ifdef(CONFIG_ST25R3911B_LIB app PRIVATE st25r3911b/st25r3911b_interrupt.c)
target_sources_ifdef(CONFIG_ST25R3911B_LIB app PRIVATE st25r3911b/st25r3911b_nfca.c)