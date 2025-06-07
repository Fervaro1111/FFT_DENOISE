AUTOMAKE_OPTIONS = subdir-objects

# name of the program
bin_PROGRAMS = bin/5G_POLAR_DEC
bin_5G_POLAR_DEC_CC = g++
# sources.
# ADD here extra source files
bin_5G_POLAR_DEC_SOURCES = ../aloe/skeleton/skeleton.c ../aloe/skeleton/params.c ../aloe/extern/rtdal_datafile.c ../standalone/test_generate.c ../src/5G_POLAR_DEC_functions.cpp ../src/5G_POLAR_DEC.c 

#pre-processor flags, e.g. includes
bin_5G_POLAR_DEC_CPPFLAGS = -D_COMPILE_ALOE -std=c++11 -I/usr/include/aff3ct-3.0.2-9-gbb649c4/aff3ct/ -I/usr/include/aff3ct-3.0.2-9-gbb649c4/cli/ -I/usr/include/aff3ct-3.0.2-9-gbb649c4/MIPP -I/usr/include/aff3ct-3.0.2-9-gbb649c4/rang -I../interfaces -I../src/ -I../aloe/skeleton -I../aloe/extern -I../../typetools/include/ -I../../../sw_api/include -I../../../utils/include
#linker flags: library paths (do not modify)
bin_5G_POLAR_DEC_LDFLAGS = -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib
#libraries to include (do not modify)
bin_5G_POLAR_DEC_LDADD = -laff3ct-3.0.2-9-gbb649c4 -lfftw3 -lm -lskeleton $(hwapi) $(swapi) -lpthread -lutils

# include these extra files in the distribution
# IMPORTANT: Add ../src/*.h files if you include headers in the src/ directory
EXTRA_DIST = ../include/*.h ../doc/ ../src/*.h ../standalone/*.h ../../../utils/include/*.h
