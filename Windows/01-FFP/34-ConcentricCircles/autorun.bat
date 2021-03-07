@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "ConcentricCircles.cpp"
rc.exe "RESOURCES.rc"
link.exe "ConcentricCircles.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "ConcentricCircles.exe"

del "ConcentricCircles.obj"
del "RESOURCES.res"
del "ConcentricCircles.exe"

EXIT (0)
