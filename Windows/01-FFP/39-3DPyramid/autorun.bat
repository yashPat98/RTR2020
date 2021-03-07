@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "3DPyramid.cpp"
rc.exe "RESOURCES.rc"
link.exe "3DPyramid.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "3DPyramid.exe"

del "3DPyramid.obj"
del "RESOURCES.res"
del "3DPyramid.exe"

EXIT (0)
