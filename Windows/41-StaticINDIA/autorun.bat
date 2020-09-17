@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "StaticINDIA.cpp"
rc.exe "RESOURCES.rc"
link.exe "StaticINDIA.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "StaticINDIA.exe"

del "StaticINDIA.obj"
del "RESOURCES.res"
del "StaticINDIA.exe"

EXIT (0)
