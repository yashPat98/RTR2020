@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Perspective.cpp"
rc.exe "RESOURCES.rc"
link.exe "Perspective.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Perspective.exe"

del "Perspective.obj"
del "RESOURCES.res"
del "Perspective.exe"

EXIT (0)
