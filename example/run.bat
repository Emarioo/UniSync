@REM @echo off

@REM working dir should not be inside example!

pushd "example\r1"
start /B CMD /c "..\..\bin\unisync"
popd
pushd "example\r2"
start /B CMD /c "..\..\bin\unisync"
popd