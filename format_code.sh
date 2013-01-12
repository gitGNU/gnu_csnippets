#!/bin/bash

has_astyle=yes
astyle --version >/dev/null || has_astyle=no

if [[ "$has_astyle" != "yes" ]]; then
	echo "Artistic Style is not installed at this site :("
	exit 1
fi

success=no
astyle --style=linux --indent=tab=8 \
	--indent-preprocessor \
	--mode=c \
	--lineend=linux \
	src/*.c csnippets/*.h && success=yes
if [[ "$success" == "yes" ]]; then
	mkdir -p orig_code
	mv src/*.orig csnippets/*.orig orig_code
fi

