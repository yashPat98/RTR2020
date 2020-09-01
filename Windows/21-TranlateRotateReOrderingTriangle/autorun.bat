@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "TranlateRotateReOrderingTriangle.cpp"
rc.exe "RESOURCES.rc"
link.exe "TranlateRotateReOrderingTriangle.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "TranlateRotateReOrderingTriangle.exe"

del "TranlateRotateReOrderingTriangle.obj"
del "RESOURCES.res"
del "TranlateRotateReOrderingTriangle.exe"

EXIT (0)
