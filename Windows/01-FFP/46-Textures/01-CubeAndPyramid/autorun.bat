@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "CubeAndPyramid.cpp"
rc.exe "RESOURCES.rc"
link.exe "CubeAndPyramid.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "CubeAndPyramid.exe"

del "CubeAndPyramid.obj"
del "RESOURCES.res"
del "CubeAndPyramid.exe"

EXIT (0)
