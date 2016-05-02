@echo off
set OLD=%CD%
cd %~dp0test
@echo on
"%CD%\love\love.exe" .
@echo off
cd %OLD%
@echo on
