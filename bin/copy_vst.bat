@echo off
setlocal

set rsync_path="E:\Documents\System\Tools\rsync\bin"
set rsync_cmd="%rsync_path%\rsync.exe" --archive

set vst2_32=/cygdrive/c/Program Files (x86)/Steinberg/VSTPlugins/radix/
set vst3_32=/cygdrive/c/Program Files (x86)/Common Files/VST3/radix/
set vst2_64=/cygdrive/c/Program Files/Steinberg/VSTPlugins/radix/
set vst3_64=/cygdrive/c/Program Files/Common Files/VST3/radix/

set categories=/cygdrive/d/Plugins/32-bit/Categories/VST2/Tools/Analyzer/Meter

call :CopyVst ^
 "%vst2_32%" ^
 "vst/K-Meter (Stereo).dll" ^
 "vst/K-Meter (Surround).dll" ^
 "vst/kmeter"

call :CopyVst ^
 "%categories%" ^
 "vst/K-Meter (Stereo).dll" ^
 "vst/K-Meter (Surround).dll" ^
 "vst/kmeter"

call :CopyVst ^
 "%vst3_32%" ^
 "vst/K-Meter (Stereo).vst3" ^
 "vst/K-Meter (Surround).vst3" ^
 "vst/kmeter"

call :CopyVst ^
 "%vst2_64%" ^
 "vst/K-Meter (Stereo x64).dll" ^
 "vst/K-Meter (Surround x64).dll" ^
 "vst/kmeter"

call :CopyVst ^
 "%vst3_64%" ^
 "vst/K-Meter (Stereo x64).vst3" ^
 "vst/K-Meter (Surround x64).vst3" ^
 "vst/kmeter"

pause
exit /b %errorlevel%


:CopyVst
set destination=%1
set dll_1=%2
set dll_2=%3
set aux_dir=%4

echo %destination%

if not exist %dll_1% exit /b 1
if not exist %dll_2% exit /b 2

%rsync_cmd% --delete %aux_dir% %destination%
%rsync_cmd%          %dll_1%   %destination%
%rsync_cmd%          %dll_2%   %destination%

exit /b 0
