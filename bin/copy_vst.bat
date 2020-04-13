@echo off
setlocal

set rsync_path="E:\Documents\System\Tools\rsync\bin"
set rsync_cmd="%rsync_path%\rsync.exe" --archive

set vst2_32=/cygdrive/c/Program Files (x86)/Steinberg/VSTPlugins/radix/
set vst3_32=/cygdrive/c/Program Files (x86)/Common Files/VST3/radix/
set vst2_64=/cygdrive/c/Program Files/Steinberg/VSTPlugins/radix/
set vst3_64=/cygdrive/c/Program Files/Common Files/VST3/radix/

set categories_32=/cygdrive/d/Plugins/32-bit/Categories/Tools/Analyzer/Meter
set categories_64=/cygdrive/d/Plugins/64-bit/Categories/Tools/Analyzer/Meter


echo.
echo "VST2 (32-bit)"
echo.

call :CopyVst ^
 "%vst2_32%" ^
 "vst/K-Meter (Stereo).dll" ^
 "vst/K-Meter (Surround).dll" ^
 "vst/kmeter"

call :CopyVst ^
 "%categories_32%" ^
 "vst/K-Meter (Stereo).dll" ^
 "vst/K-Meter (Surround).dll" ^
 "vst/kmeter"

echo.
echo "VST3 (32-bit)"
echo.

call :CopyVst ^
 "%vst3_32%" ^
 "vst3/K-Meter (Stereo).vst3" ^
 "vst3/K-Meter (Surround).vst3" ^
 "vst3/kmeter"

echo.
echo "VST2 (64-bit)"
echo.

call :CopyVst ^
 "%vst2_64%" ^
 "vst/K-Meter (Stereo x64).dll" ^
 "vst/K-Meter (Surround x64).dll" ^
 "vst/kmeter"

call :CopyVst ^
 "%categories_64%" ^
 "vst/K-Meter (Stereo x64).dll" ^
 "vst/K-Meter (Surround x64).dll" ^
 "vst/kmeter"

echo.
echo "VST3 (64-bit)"
echo.

call :CopyVst ^
 "%vst3_64%" ^
 "vst3/K-Meter (Stereo x64).vst3" ^
 "vst3/K-Meter (Surround x64).vst3" ^
 "vst3/kmeter"

echo.
pause
exit /b %errorlevel%


:CopyVst
set destination=%1
set dll_1=%2
set dll_2=%3
set aux_dir=%4

echo %destination%

%rsync_cmd% --delete %aux_dir% %destination%
%rsync_cmd%          %dll_1%   %destination%
%rsync_cmd%          %dll_2%   %destination%

exit /b 0
