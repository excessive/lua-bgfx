@echo off
set VS=C:\Program Files (x86)\Microsoft Visual Studio 12.0
set GENIE="%~dp0extern\bx\tools\bin\windows\genie.exe"
call "%VS%\VC\vcvarsall.bat" x86
set OLD=%CD%
cd %~dp0scripts
@echo on
%GENIE% vs2013
msbuild.exe lua-bgfx.sln /property:Configuration=Debug /m /t:lua-bgfx
cd %OLD%
