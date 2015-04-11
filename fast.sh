#!/bin/bash
cd build
make
read -r -p "Do you want to run the server? [Y/n] " response
case $response in
	[yY][eE][sS]|[yY])
		cd ../bin
		./beachJudge
		;;
esac
