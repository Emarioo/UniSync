@echo off
@setlocal enabledelayedexpansion

set exe=.\BetterThanBatch\bin\btb.exe
!exe! src/main -d -r

@REM set WITH_CONSOLE=1
@REM SET USE_DEBUG=1

@REM SET COMPILE_OPTIONS=/std:c++17 /EHsc /TP /MTd /nologo
@REM SET LINK_OPTIONS=/NOLOGO /INCREMENTAL:NO /IGNORE:4006 /IGNORE:4098 /IGNORE:4099

@REM SET LINK_OPTIONS=!LINK_OPTIONS! Advapi32.lib gdi32.lib shell32.lib user32.lib OpenGL32.lib ws2_32.lib
@REM SET LINK_OPTIONS=!LINK_OPTIONS! /DEBUG libs/glfw-3.3.8/lib/glfw3_mt.lib libs/glew-2.1.0/lib/glew32s.lib

@REM SET INCLUDE_DIRS=/Iinclude /Ilibs/stb/include /Ilibs/glfw-3.3.8/include /Ilibs/glew-2.1.0/include
@REM SET DEFINITIONS=/DOS_WINDOWS /FI pch.h /DGLEW_STATIC

@REM if !USE_DEBUG!==1 (
@REM     SET COMPILE_OPTIONS=!COMPILE_OPTIONS! /DEBUG /Z7
@REM     SET LINK_OPTIONS=!LINK_OPTIONS! /DEBUG
@REM )
@REM if !WITH_CONSOLE!==1 (
@REM     SET LINK_OPTIONS=!LINK_OPTIONS! /SUBSYSTEM:CONSOLE
@REM ) else (
@REM     SET LINK_OPTIONS=!LINK_OPTIONS! /SUBSYSTEM:WINDOWS
@REM ) 

@REM set /a startTime=6000*( 100%time:~3,2% %% 100 ) + 100 * ( 100%time:~6,2% %% 100 ) + ( 100%time:~9,2% %% 100 )

@REM rem ######### GATHER FILES ################
@REM SET OBJ_DIR=bin\intermediates
@REM mkdir !OBJ_DIR! 2> nul

@REM SET SRC=!OBJ_DIR!\all.cpp

@REM type nul > !SRC!
@REM for /r %%i in (*.cpp) do (
@REM     SET file=%%i
    
@REM     if "x!file:bin=!"=="x!file!" if "x!file:__=!"=="x!file!" (
@REM         if not "x!file:UniSync=!"=="x!file!" (
@REM             echo #include ^"!file:\=/!^" >> !SRC!
@REM         ) else if not "x!file:Engone=!"=="x!file!" (
@REM             echo #include ^"!file:\=/!^" >> !SRC!
            
@REM         )
@REM     )
@REM )
@REM @REM pause
@REM rem ########## COMPILE #############

@REM set /a c_startTime=6000*( 100%time:~3,2% %% 100 ) + 100* ( 100%time:~6,2% %% 100 ) + ( 100%time:~9,2% %% 100 )

@REM cl /c !COMPILE_OPTIONS! !DEFINITIONS! !INCLUDE_DIRS! !SRC! /Fo!OBJ_DIR!\all.o
@REM @REM if not %errorlevel% == 0 ( exit )

@REM set /a c_endTime=6000*(100%time:~3,2% %% 100 )+100*(100%time:~6,2% %% 100 )+(100%time:~9,2% %% 100 )

@REM rem ############# LINKING ################

@REM mkdir bin 2> nul

@REM rem Todo: link with optimizations or debug info?

@REM set /a l_startTime=6000*( 100%time:~3,2% %% 100 ) + 100* ( 100%time:~6,2% %% 100 ) + ( 100%time:~9,2% %% 100 )

@REM @REM pause
@REM @REM rc /?
@REM rc /nologo /fo bin/intermediates/resources.res UniSync\resources.rc

@REM @REM pause
@REM link !OBJ_DIR!\all.o !LINK_OPTIONS! bin/intermediates/resources.res /OUT:bin/unisync.exe
@REM @REM if not %errorlevel% == 0 ( exit )

@REM set /a l_endTime=6000*(100%time:~3,2% %% 100 )+100*(100%time:~6,2% %% 100 )+(100%time:~9,2% %% 100 )

@REM set /a endTime=6000*(100%time:~3,2% %% 100 )+100*(100%time:~6,2% %% 100 )+(100%time:~9,2% %% 100 )

@REM set /a c_finS=(c_endTime-c_startTime)/100
@REM set /a c_finS2=(c_endTime-c_startTime)%%100
@REM set /a l_finS=(l_endTime-l_startTime)/100
@REM set /a l_finS2=(l_endTime-l_startTime)%%100
@REM set /a finS=(endTime-startTime)/100
@REM set /a finS2=(endTime-startTime)%%100

@REM echo Compilation in %c_finS%.%c_finS2% seconds
@REM echo Linking in %l_finS%.%l_finS2% seconds
@REM echo Finished in %finS%.%finS2% seconds

@REM @REM bin\unisync.exe
@REM start bin\unisync.exe --file .unisync-s
@REM start bin\unisync.exe --file .unisync-c