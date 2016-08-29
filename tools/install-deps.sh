#!/bin/bash
pushd $( cd $(dirname $0) ; pwd -P )/.. > /dev/null
mkdir build
cd build
cmake -G 'Unix Makefiles' ..
./get-packages.sh
popd > /dev/null
