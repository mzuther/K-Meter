@echo off

@echo.
@premake4 --file=vs_2010.lua --os=windows vs2010

@echo.
@premake4 --cc=gcc --os=linux gmake

@echo.
@pause
