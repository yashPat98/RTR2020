@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "X-ParallelLines.cpp"
rc.exe "RESOURCES.rc"
link.exe "X-ParallelLines.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "X-ParallelLines.exe"

del "X-ParallelLines.obj"
del "RESOURCES.res"
del "X-ParallelLines.exe"

EXIT (0)
