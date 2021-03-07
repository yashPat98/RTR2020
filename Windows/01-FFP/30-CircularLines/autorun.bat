@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "CircularLines.cpp"
rc.exe "RESOURCES.rc"
link.exe "CircularLines.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "CircularLines.exe"

del "CircularLines.obj"
del "RESOURCES.res"
del "CircularLines.exe"

EXIT (0)
