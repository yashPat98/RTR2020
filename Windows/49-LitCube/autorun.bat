@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "LitCube.cpp"
rc.exe "RESOURCES.rc"
link.exe "LitCube.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "LitCube.exe"

del "LitCube.obj"
del "RESOURCES.res"
del "LitCube.exe"

EXIT (0)
