@echo off
rmdir /Q /S bin
mkdir bin
pushd bin

rem Name
set name=App

rem Include directories 
set inc=/I ..\..\..\third_party\include\ /I ..\external\

rem Source files
set src_main=..\source\main.c

rem All source together
set src_all=%src_main%

rem OS Libraries
set os_libs= opengl32.lib kernel32.lib user32.lib ^
shell32.lib vcruntime.lib msvcrt.lib gdi32.lib Advapi32.lib winmm.lib

rem Link options
set l_options=/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib

rem Compile Release
rem cl /MP /FS /Ox /W0 /Fe%name%.exe %src_all% %inc% ^
rem /EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:LIBCMT ^
rem %os_libs%

rem Compile Debug
cl /w /MP -Zi /DEBUG:FULL /Fe%name%.exe %src_all% %inc% ^
/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:LIBCMT ^
%os_libs%

popd
