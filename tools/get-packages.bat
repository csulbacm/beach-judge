@echo off
pushd %~dp0\..
set PATH=%PATH%;./build/external/python2;./build/external/nodejs
set PYTHON=%~dp0/../build/external/python2/python.exe
npm i
popd
