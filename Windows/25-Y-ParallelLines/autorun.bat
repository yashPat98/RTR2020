@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Y-ParallelLines.cpp"
rc.exe "RESOURCES.rc"
link.exe "Y-ParallelLines.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Y-ParallelLines.exe"

del "Y-ParallelLines.obj"
del "RESOURCES.res"
del "Y-ParallelLines.exe"

EXIT (0)
