@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "LookAt.cpp"
rc.exe "RESOURCES.rc"
link.exe "LookAt.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "LookAt.exe"

del "LookAt.obj"
del "RESOURCES.res"
del "LookAt.exe"

EXIT (0)
