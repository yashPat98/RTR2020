@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Triangle.cpp"
rc.exe "RESOURCES.rc"
link.exe "Triangle.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Triangle.exe"

del "Triangle.obj"
del "RESOURCES.res"
del "Triangle.exe"

EXIT (0)
