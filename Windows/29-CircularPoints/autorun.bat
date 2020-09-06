@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "CircularPoints.cpp"
rc.exe "RESOURCES.rc"
link.exe "CircularPoints.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "CircularPoints.exe"

del "CircularPoints.obj"
del "RESOURCES.res"
del "CircularPoints.exe"

EXIT (0)
