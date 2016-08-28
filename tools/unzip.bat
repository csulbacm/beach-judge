@echo off
setlocal
set InArchive=%1
set OutDir=%2
if "%OutDir%"=="" ( set OutDir=%CD% )
powershell -executionpolicy bypass -noprofile -command "Expand-Archive -Path \"%InArchive%\" -DestinationPath \"%OutDir%\" -Force -Verbose"