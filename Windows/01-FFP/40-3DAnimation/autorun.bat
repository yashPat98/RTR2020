@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "3DAnimation.cpp"
rc.exe "RESOURCES.rc"
link.exe "3DAnimation.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "3DAnimation.exe"

del "3DAnimation.obj"
del "RESOURCES.res"
del "3DAnimation.exe"

EXIT (0)
