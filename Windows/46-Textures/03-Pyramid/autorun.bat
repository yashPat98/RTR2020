@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Pyramid.cpp"
rc.exe "RESOURCES.rc"
link.exe "Pyramid.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "Pyramid.exe"

del "Pyramid.obj"
del "RESOURCES.res"
del "Pyramid.exe"

EXIT (0)
