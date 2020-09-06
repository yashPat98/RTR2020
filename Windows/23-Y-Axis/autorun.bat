@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Y-Axis.cpp"
rc.exe "RESOURCES.rc"
link.exe "Y-Axis.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Y-Axis.exe"

del "Y-Axis.obj"
del "RESOURCES.res"
del "Y-Axis.exe"

EXIT (0)
