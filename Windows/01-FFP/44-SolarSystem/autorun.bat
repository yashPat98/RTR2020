@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "SolarSystem.cpp"
rc.exe "RESOURCES.rc"
link.exe "SolarSystem.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "SolarSystem.exe"

del "SolarSystem.obj"
del "RESOURCES.res"
del "SolarSystem.exe"

EXIT (0)
