#!/bin/bash
#
# Build application with specific directives.

SOURCE_FILE=${1-'./main.cpp'}
if [ ! -f $SOURCE_FILE ]
then
	echo 'Provided invalid path to source file'
	exit 1
fi

readonly ROOT="$PWD"
readonly BUILD_DIR=$(mktemp -d)
readonly RESULT_DIRECTIVE='@result'

# Cleanup handler
function cleanup(){
	local rc=$?
	rm -r $BUILD_DIR
	echo $BUILD_DIR
	exit $rc
}
# Explored from $(man 7 signal)
# EXIT - pseudo signal (bash-generated)
trap cleanup EXIT HUP INT QUIT PIPE TERM

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

cd $BUILD_DIR
g++ $ROOT/$SOURCE_FILE -o $binary_name
if [ $? -ne 0 ]
then
	echo 'Failed to compile'
	exit 1
fi

mv $binary_name $ROOT

echo 'Done'
exit 0