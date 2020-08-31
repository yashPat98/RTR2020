@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "SpinningTriangle.cpp"
rc.exe "RESOURCES.rc"
link.exe "SpinningTriangle.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "SpinningTriangle.exe"

del "SpinningTriangle.obj"
del "RESOURCES.res"
del "SpinningTriangle.exe"

EXIT (0)
