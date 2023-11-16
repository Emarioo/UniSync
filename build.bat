@echo off
@setlocal enabledelayedexpansion

set WITH_CONSOLE=1
SET USE_DEBUG=1

SET COMPILE_OPTIONS=/std:c++17 /EHsc /TP /MTd /nologo
SET LINK_OPTIONS=/NOLOGO /INCREMENTAL:NO /IGNORE:4006 /IGNORE:4098 /IGNORE:4099

SET LINK_OPTIONS=!LINK_OPTIONS! Advapi32.lib gdi32.lib shell32.lib user32.lib OpenGL32.lib ws2_32.lib
SET LINK_OPTIONS=!LINK_OPTIONS! /DEBUG libs/glfw-3.3.8/lib/glfw3_mt.lib libs/glew-2.1.0/lib/glew32s.lib

SET INCLUDE_DIRS=/Iinclude /Ilibs/stb/include /Ilibs/glfw-3.3.8/include /Ilibs/glew-2.1.0/include
SET DEFINITIONS=/DOS_WINDOWS /FI pch.h /DGLEW_STATIC

if !USE_DEBUG!==1 (
    SET COMPILE_OPTIONS=!COMPILE_OPTIONS! /DEBUG /Z7
    SET LINK_OPTIONS=!LINK_OPTIONS! /DEBUG
)
if !WITH_CONSOLE!==1 (
    SET LINK_OPTIONS=!LINK_OPTIONS! /SUBSYSTEM:CONSOLE
) else (
    SET LINK_OPTIONS=!LINK_OPTIONS! /SUBSYSTEM:WINDOWS
) 

set /a startTime=6000*( 100%time:~3,2% %% 100 ) + 100 * ( 100%time:~6,2% %% 100 ) + ( 100%time:~9,2% %% 100 )

rem ######### GATHER FILES ################
SET OBJ_DIR=bin\intermediates
mkdir !OBJ_DIR! 2> nul

SET SRC=!OBJ_DIR!\all.cpp

type nul > !SRC!
for /r %%i in (*.cpp) do (
    SET file=%%i
    
    if "x!file:bin=!"=="x!file!" if "x!file:__=!"=="x!file!" (
        if not "x!file:UniSync=!"=="x!file!" (
            echo #include ^"!file:\=/!^" >> !SRC!
        ) else if not "x!file:Engone=!"=="x!file!" (
            echo #include ^"!file:\=/!^" >> !SRC!
            
        )
    )
)
@REM pause
rem ########## COMPILE #############

set /a c_startTime=6000*( 100%time:~3,2% %% 100 ) + 100* ( 100%time:~6,2% %% 100 ) + ( 100%time:~9,2% %% 100 )

cl /c !COMPILE_OPTIONS! !DEFINITIONS! !INCLUDE_DIRS! !SRC! /Fo!OBJ_DIR!\all.o
@REM if not %errorlevel% == 0 ( exit )

set /a c_endTime=6000*(100%time:~3,2% %% 100 )+100*(100%time:~6,2% %% 100 )+(100%time:~9,2% %% 100 )

rem ############# LINKING ################

mkdir bin 2> nul

rem Todo: link with optimizations or debug info?

set /a l_startTime=6000*( 100%time:~3,2% %% 100 ) + 100* ( 100%time:~6,2% %% 100 ) + ( 100%time:~9,2% %% 100 )

@REM pause
@REM rc /?
rc /nologo /fo bin/intermediates/resources.res UniSync\resources.rc

@REM pause
link !OBJ_DIR!\all.o !LINK_OPTIONS! bin/intermediates/resources.res /OUT:bin/unisync.exe
@REM if not %errorlevel% == 0 ( exit )

set /a l_endTime=6000*(100%time:~3,2% %% 100 )+100*(100%time:~6,2% %% 100 )+(100%time:~9,2% %% 100 )

set /a endTime=6000*(100%time:~3,2% %% 100 )+100*(100%time:~6,2% %% 100 )+(100%time:~9,2% %% 100 )

set /a c_finS=(c_endTime-c_startTime)/100
set /a c_finS2=(c_endTime-c_startTime)%%100
set /a l_finS=(l_endTime-l_startTime)/100
set /a l_finS2=(l_endTime-l_startTime)%%100
set /a finS=(endTime-startTime)/100
set /a finS2=(endTime-startTime)%%100

echo Compilation in %c_finS%.%c_finS2% seconds
echo Linking in %l_finS%.%l_finS2% seconds
echo Finished in %finS%.%finS2% seconds

bin\unisync.exe