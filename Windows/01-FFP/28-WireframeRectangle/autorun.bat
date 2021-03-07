@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "WireframeRectangle.cpp"
rc.exe "RESOURCES.rc"
link.exe "WireframeRectangle.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "WireframeRectangle.exe"

del "WireframeRectangle.obj"
del "RESOURCES.res"
del "WireframeRectangle.exe"

EXIT (0)
