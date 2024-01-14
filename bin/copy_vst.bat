@echo off

rem  ----------------------------------------------------------------------------
rem  
rem  K-Meter
rem  =======
rem  Implementation of a K-System meter according to Bob Katz' specifications
rem  
rem  Copyright (c) 2010-2024 Martin Zuther (http://www.mzuther.de/)
rem  
rem  This program is free software: you can redistribute it and/or modify
rem  it under the terms of the GNU General Public License as published by
rem  the Free Software Foundation, either version 3 of the License, or
rem  (at your option) any later version.
rem  
rem  This program is distributed in the hope that it will be useful,
rem  but WITHOUT ANY WARRANTY; without even the implied warranty of
rem  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem  GNU General Public License for more details.
rem  
rem  You should have received a copy of the GNU General Public License
rem  along with this program.  If not, see <http://www.gnu.org/licenses/>.
rem  
rem  Thank you for using free software!
rem  
rem  ----------------------------------------------------------------------------


rem  ############################################################################
rem
rem  WARNING: this file was auto-generated, please do not edit!
rem
rem  ############################################################################

setlocal

set rsync_path="E:\Documents\System\Tools\rsync\bin"
set rsync_cmd="%rsync_path%\rsync.exe" --archive

set vst2_32=/cygdrive/c/Program Files (x86)/Steinberg/VSTPlugins/mzuther/
set vst3_32=/cygdrive/c/Program Files (x86)/Common Files/VST3/mzuther/
set vst2_64=/cygdrive/c/Program Files/Steinberg/VSTPlugins/mzuther/
set vst3_64=/cygdrive/c/Program Files/Common Files/VST3/mzuther/

set vst2_32_categories="/cygdrive/d/Plugins/32-bit/Categories/Tools/Analyzer/Meter"
set vst2_64_categories="/cygdrive/d/Plugins/64-bit/Categories/Tools/Analyzer/Meter"


echo.
echo VST2 (32 bit)
echo.

call :CopyVst       "FFTW/" "%vst2_32%/FFTW"
call :CopyVst       "vst2/K-Meter.dll" "%vst2_32%"
call :CopyVst       "vst2/K-Meter (surround).dll" "%vst2_32%"
call :CopyVst       "vst2/Documentation/K-Meter.pdf" "%vst2_32%"

echo.
echo VST2 (32 bit, Categories)
echo.

call :CopyVst       "FFTW/" "%vst2_32_categories%/FFTW"
call :CopyVst       "vst2/K-Meter.dll" "%vst2_32_categories%"
call :CopyVst       "vst2/K-Meter (surround).dll" "%vst2_32_categories%"
call :CopyVst       "vst2/Documentation/K-Meter.pdf" "%vst2_32_categories%"


echo.
echo VST3 (32 bit)
echo.

call :CopyVst       "FFTW/" "%vst3_32%/Resources/FFTW"
call :CopyVstDelete "vst3/K-Meter.vst3" "%vst3_32%"


echo.
echo VST2 (64 bit)
echo.

call :CopyVst       "FFTW/" "%vst2_64%/FFTW"
call :CopyVst       "vst2/K-Meter x64.dll" "%vst2_64%"
call :CopyVst       "vst2/K-Meter (surround) x64.dll" "%vst2_64%"
call :CopyVst       "vst2/Documentation/K-Meter.pdf" "%vst2_64%"

echo.
echo VST2 (64 bit, Categories)
echo.

call :CopyVst       "FFTW/" "%vst2_64_categories%/FFTW"
call :CopyVst       "vst2/K-Meter x64.dll" "%vst2_64_categories%"
call :CopyVst       "vst2/K-Meter (surround) x64.dll" "%vst2_64_categories%"
call :CopyVst       "vst2/Documentation/K-Meter.pdf" "%vst2_64_categories%"


echo.
echo VST3 (64 bit)
echo.

call :CopyVst       "FFTW/" "%vst3_64%/Resources/FFTW"
call :CopyVstDelete "vst3/K-Meter.vst3" "%vst3_64%"


echo.
pause
exit /b %errorlevel%


:CopyVst
set source=%~1
set destination=%~2

echo CopyVst       %source%
%rsync_cmd% "%source%" "%destination%"

exit /b 0


:CopyVstDelete
set source=%~1
set destination=%~2

echo CopyVstDelete %source%
%rsync_cmd% --delete --exclude="*-linux" "%source%" "%destination%"

exit /b 0
