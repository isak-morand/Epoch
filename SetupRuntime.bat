@echo off
pushd %~dp0
python scripts\setup.py --target=runtime
PAUSE