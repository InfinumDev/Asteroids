@echo off

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl -Zi -nologo ..\Asteroids\code\win32_asteroids.cpp user32.lib gdi32.lib
popd