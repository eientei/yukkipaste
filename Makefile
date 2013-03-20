#### VARS ####

RMRF = rm -rf
MKDIRP = mkdir -p
CD = cd
CMAKE = cmake
CMAKE_OPTS = 
CMAKE_ROOT = "../"

#### PARAMETERS ####

CMAKE_BUILD = build
PROJECT_LIBDIR = libs

#### TARGETS ####

.PHONY: clean crossbuilds

all: libs make

crossbuilds:
	for chen in cmake/Toolchains/*; do \
		name="$$(echo $$chen | sed 's|.*/\([^-]*\)-.*|\1|')"; \
		mkdir -p "crossbuilds/$$name"; \
		make -C . all \
			CMAKE_OPTS=-DCMAKE_TOOLCHAIN_FILE="$$(pwd)/$$chen" \
			CMAKE_BUILD="crossbuilds/$$name" \
			CMAKE_ROOT="../../"; \
	done

crossbuilds-clean:
	$(RMRF) "crossbuilds" 

clean: main-clean libs-clean crossbuilds-clean

make: main

libs:

libs-clean:

#### MAIN ####

main: main-configure main-make

main-configure:
	$(MKDIRP) ${CMAKE_BUILD}
	$(CD) ${CMAKE_BUILD} && $(CMAKE) ${CMAKE_OPTS} ${CMAKE_ROOT}

main-make:
	$(CD) ${CMAKE_BUILD} && $(MAKE)

main-clean:
	$(RMRF) ${CMAKE_BUILD}
