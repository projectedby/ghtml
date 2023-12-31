# ./Makefile.am:

AM_CXXFLAGS     = -I/usr/include/libxml2        \
                  -I/usr/lib/llvm-14/include    \
                  -std=c++14                    \
                  -fno-exceptions               \
                  -D_GNU_SOURCE                 \
                  -D__STDC_CONSTANT_MACROS      \
                  -D__STDC_FORMAT_MACROS        \
                  -D__STDC_LIMIT_MACROS

bin_PROGRAMS    =   ghtml

LIBS            = -lxml2 -lLLVM-14 -g

ghtml_SOURCES   =   ./ghtml.cc
