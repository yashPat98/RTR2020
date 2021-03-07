@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Shapes.cpp"
rc.exe "RESOURCES.rc"
link.exe "Shapes.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "Shapes.exe"

del "Shapes.obj"
del "RESOURCES.res"
del "Shapes.exe"

EXIT (0)
