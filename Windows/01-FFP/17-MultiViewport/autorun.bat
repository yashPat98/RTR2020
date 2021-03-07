@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "MultiViewport.cpp"
rc.exe "RESOURCES.rc"
link.exe "MultiViewport.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "MultiViewport.exe"

del "MultiViewport.obj"
del "RESOURCES.res"
del "MultiViewport.exe"

EXIT (0)
