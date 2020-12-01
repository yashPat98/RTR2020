@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "LightsAndMaterial.cpp"
rc.exe "RESOURCES.rc"
link.exe "LightsAndMaterial.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "LightsAndMaterial.exe"

del "LightsAndMaterial.obj"
del "RESOURCES.res"
del "LightsAndMaterial.exe"

EXIT (0)
