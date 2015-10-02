#!/bin/bash

exec 3< <(./tools/buildsock)

cd bin
while true
do
	#read -r -p "Continue? [*/n] " response
	#case $response in
	#	[nN][oO]|[nN])
	#		continue
	#		;;
	#esac
	./beachJudge & pid=$!
#	read -r -p "" response
	read <&3 dummy
#	echo "$dummy"
	kill $pid
done
