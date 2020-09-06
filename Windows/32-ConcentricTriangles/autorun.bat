@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "ConcentricTriangles.cpp"
rc.exe "RESOURCES.rc"
link.exe "ConcentricTriangles.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "ConcentricTriangles.exe"

del "ConcentricTriangles.obj"
del "RESOURCES.res"
del "ConcentricTriangles.exe"

EXIT (0)
