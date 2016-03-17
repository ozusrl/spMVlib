#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<spMVgen parameters>'" >&2
  exit 1
fi

methodName=$1
methodParam1=$2
methodParam2=$3

test() {
    local matrixName=$1
    local methodName=$2
    local methodParam1=$3
    local methodParam2=$4
    folderName=../../newspecs/spMVgen/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
    ./spMVgen matrices/$matrixName $methodName $methodParam1 $methodParam2 -debug > "$folderName"/output.txt
}

echo "*" Running spMVlib for "$methodName""$methodParam1""$methodParam2"

while read matrixName
do
    echo -n $matrixName" "
    folderName=../../../newspecs/spMVgen/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
    mkdir -p "$folderName"
    rm -f "$folderName"/output.txt
    cd ..
    test $matrixName "$methodName" "$methodParam1" "$methodParam2"
    cd - > /dev/null
done < matrixNames.txt

echo " "