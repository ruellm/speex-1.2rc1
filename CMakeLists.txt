cmake_minimum_required(VERSION 3.17)
project(speex_1_2rc1)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFLOATING_POINT -DEXPORT=''")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFLOATING_POINT -DEXPORT=''")

include_directories(
        include
        libspeex
)

add_library(speex_1_2rc1 STATIC
      speex.cpp
      adapter.cpp
      libspeex/cb_search.c
      libspeex/exc_10_32_table.c
      libspeex/exc_8_128_table.c
      libspeex/filters.c
      libspeex/gain_table.c
      libspeex/hexc_table.c
      libspeex/high_lsp_tables.c
      libspeex/lsp.c
      libspeex/ltp.c
      libspeex/speex.c
      libspeex/stereo.c
      libspeex/vbr.c
      libspeex/vq.c
      libspeex/bits.c
      libspeex/exc_10_16_table.c
      libspeex/exc_20_32_table.c
      libspeex/exc_5_256_table.c
      libspeex/exc_5_64_table.c
      libspeex/gain_table_lbr.c
      libspeex/hexc_10_32_table.c
      libspeex/lpc.c
      libspeex/lsp_tables_nb.c
      libspeex/modes.c
      libspeex/modes_wb.c
      libspeex/nb_celp.c
      libspeex/quant_lsp.c
      libspeex/sb_celp.c
      libspeex/speex_callbacks.c
      libspeex/speex_header.c
      libspeex/window.c adapter.cpp adapter.h)