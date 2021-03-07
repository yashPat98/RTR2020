@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Point.cpp"
rc.exe "RESOURCES.rc"
link.exe "Point.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Point.exe"

del "Point.obj"
del "RESOURCES.res"
del "Point.exe"

EXIT (0)
