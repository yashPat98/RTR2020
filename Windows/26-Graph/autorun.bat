@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Graph.cpp"
rc.exe "RESOURCES.rc"
link.exe "Graph.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Graph.exe"

del "Graph.obj"
del "RESOURCES.res"
del "Graph.exe"

EXIT (0)
