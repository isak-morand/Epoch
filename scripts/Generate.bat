@echo off
pushd %~dp0
call ..\vendor\Premake\premake5.exe
popd
PAUSE