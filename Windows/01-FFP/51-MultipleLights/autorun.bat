@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "MultipleLights.cpp"
rc.exe "RESOURCES.rc"
link.exe "MultipleLights.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "MultipleLights.exe"

del "MultipleLights.obj"
del "RESOURCES.res"
del "MultipleLights.exe"

EXIT (0)
