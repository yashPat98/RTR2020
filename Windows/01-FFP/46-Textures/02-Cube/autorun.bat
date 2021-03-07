@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Cube.cpp"
rc.exe "RESOURCES.rc"
link.exe "Cube.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "Cube.exe"

del "Cube.obj"
del "RESOURCES.res"
del "Cube.exe"

EXIT (0)
