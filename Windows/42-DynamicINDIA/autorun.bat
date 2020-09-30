@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "DynamicINDIA.cpp"
rc.exe "RESOURCES.rc"
link.exe "DynamicINDIA.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

del "DynamicINDIA.obj"
del "RESOURCES.res"

EXIT (0)
