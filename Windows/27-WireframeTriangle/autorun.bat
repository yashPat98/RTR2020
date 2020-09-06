@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "WireframeTriangle.cpp"
rc.exe "RESOURCES.rc"
link.exe "WireframeTriangle.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "WireframeTriangle.exe"

del "WireframeTriangle.obj"
del "RESOURCES.res"
del "WireframeTriangle.exe"

EXIT (0)
