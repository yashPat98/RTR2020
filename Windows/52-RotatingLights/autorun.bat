@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "RotatingLights.cpp"
rc.exe "RESOURCES.rc"
link.exe "RotatingLights.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "RotatingLights.exe"

del "RotatingLights.obj"
del "RESOURCES.res"
del "RotatingLights.exe"

EXIT (0)
