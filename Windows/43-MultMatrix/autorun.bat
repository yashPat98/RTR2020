@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "MultMatrix.cpp"
rc.exe "RESOURCES.rc"
link.exe "MultMatrix.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "MultMatrix.exe"

del "MultMatrix.obj"
del "RESOURCES.res"
del "MultMatrix.exe"

EXIT (0)
