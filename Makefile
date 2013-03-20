#### VARS ####

RMRF = rm -rf
MKDIRP = mkdir -p
CD = cd
CMAKE = cmake

#### PARAMETERS ####

CMAKE_BUILD = build
PROJECT_LIBDIR = libs

#### TARGETS ####

all: libs make

clean: main-clean libs-clean

make: main

libs:

libs-clean:

#### MAIN ####

main: main-configure main-make

main-configure:
	$(MKDIRP) ${CMAKE_BUILD}
	$(CD) ${CMAKE_BUILD} && $(CMAKE) ../

main-make:
	$(CD) ${CMAKE_BUILD} && $(MAKE)

main-clean:
	$(RMRF) ${CMAKE_BUILD}
