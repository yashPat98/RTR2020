@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "X-Axis.cpp"
rc.exe "RESOURCES.rc"
link.exe "X-Axis.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "X-Axis.exe"

del "X-Axis.obj"
del "RESOURCES.res"
del "X-Axis.exe"

EXIT (0)
