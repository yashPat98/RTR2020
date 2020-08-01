@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "MyWindowEvents.cpp"
link.exe "MyWindowEvents.obj" "user32.lib" "gdi32.lib"

call "MyWindowEvents.exe"

del "MyWindowEvents.obj"
del "MyWindowEvents.exe"

EXIT (0)
