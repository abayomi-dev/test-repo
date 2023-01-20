@echo off
rem This help run the makefile for various builds e.g tmake pax

echo Compiling for Full Package
type makefiles\makefile > makefile
nmake
del makefile
echo BUILD TIME: %time%
goto :eof

:all
echo Compiling for Regular
type makefiles\regular\makefile > makefile
nmake
del makefile
echo BUILD TIME: %time%
TIMEOUT /T 10
echo Compiling for Hotels
type makefiles\hotels\makefile > makefile
nmake
del makefile
echo BUILD TIME: %time%
TIMEOUT /T 10
echo Compiling for Full Package
type makefiles\full\makefile > makefile
nmake
del makefile
echo BUILD TIME: %time%
TIMEOUT /T 10
echo BUILD TIME: %time%
goto :eof

				
echo %1 does not exist
goto :eof

:help
echo examples tmake

exit /B 1
