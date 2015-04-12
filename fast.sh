#!/bin/bash
#   1. Run as './fast.sh; fg' with vim in the background
#   2. ????
#   3. PROFIT!!!
cd build
make
read -r -p "Do you want to run the server? [*/n] " response
case $response in
	[nN][oO]|[nN])
		;;
	*)
		cd ../bin
		./beachJudge
		;;
esac
