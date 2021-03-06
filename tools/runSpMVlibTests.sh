#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 '<thundercat parameters>'" >&2
  exit 1
fi

if [ -z ${MATRICES+x} ]; then
    echo "Set MATRICES variable to the matrices folder."
    exit 1
fi

source /opt/intel/bin/compilervars.sh intel64

methodName=$1
methodParam1=$2
methodParam2=$3

echo "Running spMVlib test " $methodName $methodParam1 $methodParam2

while read line
do
    IFS=' ' read -r -a info <<< "$line"
    groupName=${info[0]}
    matrixName=${info[1]}

    echo -n "$groupName"/"$matrixName "

    cd ..
    folderName=data/"$groupName"/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
    mkdir -p "$folderName"
    rm -f "$folderName"/runtime.txt

    ./build/thundercat $MATRICES/$groupName/$matrixName/$matrixName $methodName $methodParam1 $methodParam2 | grep "perIteration" | awk '{print $2}' >> "$folderName"/runtime.txt
    ./build/thundercat $MATRICES/$groupName/$matrixName/$matrixName $methodName $methodParam1 $methodParam2 | grep "perIteration" | awk '{print $2}' >> "$folderName"/runtime.txt
    ./build/thundercat $MATRICES/$groupName/$matrixName/$matrixName $methodName $methodParam1 $methodParam2 | grep "perIteration" | awk '{print $2}' >> "$folderName"/runtime.txt
    cd tools

done < matrixNames.txt

echo " "

findMins() {
    currentTime=`date +%Y.%m.%d_%H.%M`
    local fileName=../data/$HOSTNAME.thundercat."$methodName""$methodParam1""$methodParam2".$currentTime.csv 
    rm -f $fileName
    while read line
    do
        IFS=' ' read -r -a info <<< "$line"
        groupName=${info[0]}
        matrixName=${info[1]}

	cd ../data/"$groupName"/"$matrixName"/"$methodName""$methodParam1""$methodParam2"
	runTimes=`cat runtime.txt`
	cd - > /dev/null
	echo -n $groupName" "$matrixName" "  >> $fileName
	./findMinTiming.py $runTimes >> $fileName
	echo     ""  >> $fileName
    done < matrixNames.txt
}

findMins

echo "$methodName $methodParam1 $methodParam2 test on $HOSTNAME has finished." | mail -s "$methodName $methodParam1 $methodParam2 test on $HOSTNAME" baris.aktemur@ozyegin.edu.tr
