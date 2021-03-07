@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "2D-Animation.cpp"
rc.exe "RESOURCES.rc"
link.exe "2D-Animation.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "2D-Animation.exe"

del "2D-Animation.obj"
del "RESOURCES.res"
del "2D-Animation.exe"

EXIT (0)
