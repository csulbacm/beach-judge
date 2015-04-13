#!/bin/bash

cd build
while true
do
	echo "Building..."
	make
	read -r -p "Continue? [*/n] " response
	case $response in
		[nN][oO]|[nN])
			continue
			;;
	esac
	cd ../bin
	./beachJudge & pid=$!
	read -r -p "" response
	kill $pid
	cd ../build
done
