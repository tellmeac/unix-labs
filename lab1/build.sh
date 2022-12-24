#!/bin/bash
#
# Build application with specific directives.

readonly ROOT="$PWD"
readonly BUILD_DIR=$(mktemp -d build.XXX)
readonly SOURCE_FILE='./main.cpp'
readonly RESULT_DIRECTIVE='@result'

# Cleanup handler
function cleanup(){
	local rc=$?

	[ -d $BUILD_DIR ] && rm -r $BUILD_DIR
	exit $rc
}
# Explored from $(man 7 signal)
trap cleanup EXIT SIGINT

# Extracts directive value from source file
# Globals:
# 	SOURCE_FILE
# Arguments:
# 	name
# Outputs:
# 	Writes directive value to stdout
extract_directive_value() {
	local name=$1

	local line="$(grep $name $SOURCE_FILE)"
	if [ $? -ne 0 ]
	then
		return 1
	fi

	echo "$(echo "$line" | cut -d ':' -f 2)"
	return 0
}

binary_name="$(extract_directive_value $RESULT_DIRECTIVE $source_file)"
if [ $? -ne 0 ]
then 
	echo 'Failed to resolve destionation path'
	exit 1
fi

binary_path=$BUILD_DIR/$binary_name
make RESULT=$binary_path
cp -f $binary_path $ROOT

echo 'Done'
exit 0