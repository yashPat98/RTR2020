@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "DirectFullscreen.cpp"
link.exe "DirectFullscreen.obj" "user32.lib" "gdi32.lib"

call "DirectFullscreen.exe"

del "DirectFullscreen.obj"
del "DirectFullscreen.exe"

EXIT (0)
