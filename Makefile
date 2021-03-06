#### VARS ####

RMRF = rm -rf
MKDIRP = mkdir -p
CD = cd
CMAKE = cmake
CMAKE_ROOT = "../"
DESTDIR = /usr
INSTALLDIR = /usr
CMAKE_OPTS =

#### PARAMETERS ####

CMAKE_BUILD = build
PROJECT_LIBDIR = libs
VERSION = 1.1

#### TARGETS ####

.PHONY: clean crossbuilds

all: libs make

crossbuilds:
	for chen in cmake/Cross/*; do \
		name="$$(echo $$chen | sed 's|.*/\([^-]*\)-.*|\1|')"; \
		mkdir -p "crossbuilds/$$name"; \
		make -C . install \
			DESTDIR="../../crossout/${VERSION}/$$name" \
			CMAKE_OPTS=-DCMAKE_TOOLCHAIN_FILE="$$(pwd)/$$chen" \
			CMAKE_BUILD="crossbuilds/$$name" \
			CMAKE_ROOT="../../"; \
	done

crossbuilds-clean:
	$(RMRF) "crossbuilds" 
	$(RMRF) "crossout" 

clean: main-clean libs-clean crossbuilds-clean

make: main

install: all
	$(CD) ${CMAKE_BUILD} && $(MAKE) install

libs:

libs-clean:

#### MAIN ####

main:
	$(MKDIRP) ${CMAKE_BUILD}
	$(CD) ${CMAKE_BUILD} && \
		$(CMAKE) ${CMAKE_OPTS} -DCMAKE_INSTALL_PREFIX=${INSTALLDIR}  ${CMAKE_ROOT}
	$(CD) ${CMAKE_BUILD} && $(MAKE)

main-clean:
	$(RMRF) ${CMAKE_BUILD}
