@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Checkerboard.cpp"
rc.exe "RESOURCES.rc"
link.exe "Checkerboard.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "Checkerboard.exe"

del "Checkerboard.obj"
del "RESOURCES.res"
del "Checkerboard.exe"

EXIT (0)
