rem Compile static lib

@echo off
rmdir /Q /S lib
mkdir lib
pushd lib

rem Name
set name=App

rem Include directories 
set inc=/I ..\..\third_party\include\

rem Source files
set src_main=..\source\impl.c

rem All source together
set src_all=%src_main%

rem Compile objects
cl /c /MP /GS /FS /Ox -W1 -Oi -Gm- /Ob2 /GR /Gd /MD %src_all% %inc% /EHsc

rem Compile Static Lib
lib *obj /out:gs.lib

popd

rem Compile main example

@echo off
rmdir /Q /S bin
mkdir bin
pushd bin

rem Name
set name=App

rem Include directories 
set inc=/I ..\..\third_party\include\

rem Source files
set src_main=..\source\main.c

rem Library directories
set lib_d=/LIBPATH:"..\lib\"

rem OS Libraries
set os_libs= opengl32.lib kernel32.lib user32.lib ^
shell32.lib vcruntime.lib msvcrt.lib gdi32.lib Winmm.lib Advapi32.lib

rem User Libraries
set libs=gs.lib

rem Link options
set l_options=/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib

rem Compile
cl /MP /FS /Ox /W1 /Fe%name%.exe %src_main% %inc% ^
/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:LIBCMT ^
%lib_d% %libs% %os_libs%

popd
