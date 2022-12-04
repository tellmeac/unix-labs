#!/bin/bash

SCRIPT_DIR=" $( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
WORK_DIR=$(mktemp -d)

trap exit_handler EXIT HUP INT QUIT PIPE TERM
function exit_handler(){
	local rc=$?
	[ -d $WORK_DIR ] && rm -r $WORK_DIR
	exit $rc
}

RESULT_DIRECTIVE='// @result:'
DEFAULT_RESULT=main.out

if (gcc --version)
then
	if grep "$RESULT_DIRECTIVE" main.cpp
	then
		compile_path=$(grep "$RESULT_DIRECTIVE" main.cpp)
		DEFAULT_RESULT=$(echo "$compile_path" | cut -d ':' -f 2)
    else
        echo "result filename should be specified with @result commentary in source, using default"
	fi
	make RESULT=$WORK_DIR/$DEFAULT_RESULT
else
	echo "gcc is not installed"
	exit 1
fi

cp -f $WORK_DIR/$DEFAULT_RESULT $SCRIPT_DIR
rm -r $WORK_DIR