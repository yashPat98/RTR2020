@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "RoboticArm.cpp"
rc.exe "RESOURCES.rc"
link.exe "RoboticArm.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "RoboticArm.exe"

del "RoboticArm.obj"
del "RESOURCES.res"
del "RoboticArm.exe"

EXIT (0)
