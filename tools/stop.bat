@echo off
pushd %~dp0\..
set PATH=%PATH%;./build/external/nodejs
npm run stop
popd