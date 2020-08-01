@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Fullscreen.cpp"
link.exe "Fullscreen.obj" "user32.lib" "gdi32.lib"

call "Fullscreen.exe"

del "Fullscreen.obj"
del "Fullscreen.exe"

EXIT (0)
