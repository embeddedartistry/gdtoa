# This makefile is used to provide convenient access to the generated  makefile

# Export all args to act as a passthrough
export

all: build

.PHONY: build_gen
build_gen:
	@build/bin/premake5 --file=premake5.lua gmake

.PHONY: arithchk
arithchk: build_gen
	@make -C build/gen arithchk

.PHONY: gdtoa
gdtoa: arithchk
	@make -C build/gen/ gdtoa

.PHONY: build
build: build_gen
	@make -C build/gen/

.PHONY: help
help: build_gen
	@make -C build/gen/ help

.PHONY: clean
clean:
ifeq ("$(wildcard build/gen/)","")
	@echo "No generated build files: skipping clean"
else
	@make -C build/gen/ clean
endif

.PHONY: purify
purify: clean
	@rm -rf build/gen

.PHONY: regen
regen:
	@build/bin/premake5 --file=premake5.lua gmake

.PHONY: format
format:
	@tools/format/clang-format-all.sh
