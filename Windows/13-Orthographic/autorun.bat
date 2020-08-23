@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Orthographic.cpp"
rc.exe "RESOURCES.rc"
link.exe "Orthographic.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Orthographic.exe"

del "Orthographic.obj"
del "RESOURCES.res"
del "Orthographic.exe"

EXIT (0)
