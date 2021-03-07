@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "ConcentricRectangles.cpp"
rc.exe "RESOURCES.rc"
link.exe "ConcentricRectangles.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "ConcentricRectangles.exe"

del "ConcentricRectangles.obj"
del "RESOURCES.res"
del "ConcentricRectangles.exe"

EXIT (0)
