@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "24Spheres.cpp"
rc.exe "RESOURCES.rc"
link.exe "24Spheres.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "24Spheres.exe"

del "24Spheres.obj"
del "RESOURCES.res"
del "24Spheres.exe"

EXIT (0)
