#!/bin/bash

cd bin
while true
do
	read -r -p "Continue? [*/n] " response
	case $response in
		[nN][oO]|[nN])
			continue
			;;
	esac
	./beachJudge & pid=$!
	read -r -p "" response
	kill $pid
done
