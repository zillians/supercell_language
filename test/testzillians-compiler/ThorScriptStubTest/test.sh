#!/bin/sh

TEMP_FILE_A=`mktemp`
TEMP_FILE_B=`mktemp`
TEMP_FILE_C=`mktemp`

TS_COMPILE=$1
TS_STUB=$2
INPUT=$3
STUB_TYPE=$4
GOLD_OUTPUT=$5
FORMAT_SH=$6

TEMP_FILE_A="${TEMP_FILE_A}.ast"
$TS_COMPILE $INPUT --root-dir=`dirname $INPUT` --emit-ast=$TEMP_FILE_A
$TS_STUB $TEMP_FILE_A --stub-type=$STUB_TYPE --game-name=DummyGameName --translator-uuid=DummyTranslatorUUID --module-uuid=DummyModuleUUID --stdout > $TEMP_FILE_B
$FORMAT_SH $TEMP_FILE_B

cat $GOLD_OUTPUT > $TEMP_FILE_C
diff $TEMP_FILE_B $TEMP_FILE_C
ERROR_CODE="$?"

if [ $ERROR_CODE -ne 0 ];
then
    echo "fail!"
    exit 1
fi
echo "success!"
exit 0
