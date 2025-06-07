AUTOMAKE_OPTIONS = subdir-objects

# name of the program
bin_PROGRAMS = bin/CHANNEL_NOISE_REPORT

# sources.
# ADD here extra source files
bin_CHANNEL_NOISE_REPORT_SOURCES = ../aloe/skeleton/skeleton.c ../aloe/skeleton/params.c ../aloe/extern/rtdal_datafile.c ../standalone/test_generate.c ../src/CHANNEL_NOISE_REPORT_functions.c ../src/CHANNEL_NOISE_REPORT.c 

#pre-processor flags, e.g. includes
bin_CHANNEL_NOISE_REPORT_CPPFLAGS = -D_COMPILE_ALOE -I../interfaces -I../src/ -I../aloe/skeleton -I../aloe/extern -I../../typetools/include/ -I../../../sw_api/include
#linker flags: library paths (do not modify)
bin_CHANNEL_NOISE_REPORT_LDFLAGS = -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib
#libraries to include (do not modify)
bin_CHANNEL_NOISE_REPORT_LDADD =  -lfftw3 -lskeleton $(hwapi) $(swapi) 

# include these extra files in the distribution
# IMPORTANT: Add ../src/*.h files if you include headers in the src/ directory
EXTRA_DIST = ../include/*.h ../doc/ ../src/*.h ../standalone/*.h
