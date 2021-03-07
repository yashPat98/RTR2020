@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Transformations.cpp"
rc.exe "RESOURCES.rc"
link.exe "Transformations.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Transformations.exe"

del "Transformations.obj"
del "RESOURCES.res"
del "Transformations.exe"

EXIT (0)
