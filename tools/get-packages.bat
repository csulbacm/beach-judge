@echo off
echo %cd%
pushd %~dp0\..
echo %cd%
set PATH=%PATH%;./build/external/nodejs
npm i
popd