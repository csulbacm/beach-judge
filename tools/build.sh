#!/bin/bash
pushd $( cd $(dirname $0) ; pwd -P )/.. > /dev/null
export PATH=$PATH:'./build/external/nodejs/bin'
npm run build
popd > /dev/null
