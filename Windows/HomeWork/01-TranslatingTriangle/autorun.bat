@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "TranslatingTriangle.cpp"
rc.exe "RESOURCES.rc"
link.exe "TranslatingTriangle.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "TranslatingTriangle.exe"

del "TranslatingTriangle.obj"
del "RESOURCES.res"
del "TranslatingTriangle.exe"

EXIT (0)
