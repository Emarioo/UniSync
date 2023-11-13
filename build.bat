@echo off
@setlocal enabledelayedexpansion

SET COMPILE_OPTIONS=/DEBUG /std:c++14 /EHsc /TP /Z7 /MTd /nologo
SET LINK_OPTIONS=/IGNORE:4006 /DEBUG /SUBSYSTEM:CONSOLE /NOLOGO

SET INCLUDE_DIRS=/Iinclude /Ilibs/GLEW/include /Ilibs/GLFW/include /Ilibs/include
SET DEFINITIONS=/DENGONE_LOGGER /DENGONE_TRACKER /DENGONE_OPENGL
SET FORCE_INCLUDES=
SET LIBRARIES=Engone.lib
SET LIBRARY_DIRS=

@rem DO NOT TURN OFF USE_LOCAL. DrawingStudio uses an old version of Engone.
SET USE_LOCAL=1

if !USE_LOCAL! == 1 (
    SET INCLUDE_DIRS=!INCLUDE_DIRS! /Ilibs\Engone\include
    SET LIBRARY_DIRS=!LIBRARY_DIRS! /LIBPATH:libs\Engone\lib
) else (
    @REM exist "D:\Backup\CodeProjects\ProjectUnknown\Engone\include\"
    @REM exist "D:\Backup\CodeProjects\ProjectUnknown\bin\Engone\Debug-MSVC\Engone.lib"
    SET INCLUDE_DIRS=!INCLUDE_DIRS! /ID:\Backup\CodeProjects\ProjectUnknown\Engone\include
    echo d | xcopy /y "D:\Backup\CodeProjects\ProjectUnknown\include\Engone" "libs\Engone\include\Engone" /s /d > nul
    SET LIBRARY_DIRS=!LIBRARY_DIRS! /LIBPATH:D:\Backup\CodeProjects\ProjectUnknown\bin\Engone\Debug-MSVC
    echo f | xcopy /y "D:\Backup\CodeProjects\ProjectUnknown\bin\Engone\Debug-MSVC\Engone.lib" "libs\Engone\lib\Engone.lib" /d > nul
)

set /a startTime=6000*( 100%time:~3,2% %% 100 ) + 100 * ( 100%time:~6,2% %% 100 ) + ( 100%time:~9,2% %% 100 )

rem ######### GATHER FILES ################
SET OBJ_DIR=bin\intermediates
mkdir !OBJ_DIR! 2> nul

SET SRC=!OBJ_DIR!\all.cpp

type nul > !SRC!
for /r %%i in (*.cpp) do (
    SET file=%%i
    
    if not "x!file:UniSync=!"=="x!file!" if "x!file:bin=!"=="x!file!" (
        echo #include ^"!file:\=/!^" >> !SRC!
    )
)
@REM pause
rem ########## COMPILE #############

set /a c_startTime=6000*( 100%time:~3,2% %% 100 ) + 100* ( 100%time:~6,2% %% 100 ) + ( 100%time:~9,2% %% 100 )

cl /c !COMPILE_OPTIONS! !FORCE_INCLUDES! !DEFINITIONS! !INCLUDE_DIRS! !SRC! /Fo!OBJ_DIR!\all.o
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
link !OBJ_DIR!\all.o !LIBRARY_DIRS! !LIBRARIES! !LINK_OPTIONS! bin/intermediates/resources.res /OUT:bin/UniSync.exe
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

@REM start bin\UniSync.exe