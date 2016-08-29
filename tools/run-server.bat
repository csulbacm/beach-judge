@echo off
%~dp0/../build/external/nodejs/node.exe %~dp0/../build/node/index.js
echo %errorLevel%
if errorlevel 1 (
	pause
)
exit