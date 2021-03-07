@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "PitchingRectangle.cpp"
rc.exe "RESOURCES.rc"
link.exe "PitchingRectangle.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "PitchingRectangle.exe"

del "PitchingRectangle.obj"
del "RESOURCES.res"
del "PitchingRectangle.exe"

EXIT (0)
