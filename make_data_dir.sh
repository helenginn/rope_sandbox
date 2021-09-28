#!/bin/bash

if [ $# -eq 0 ]; then
	exit
fi

echo "#include $MESON_SOURCE_ROOT/paths.h"


