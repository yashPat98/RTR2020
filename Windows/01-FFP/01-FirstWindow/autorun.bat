@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "MyWindow.cpp"
link.exe "MyWindow.obj" "user32.lib" "gdi32.lib"

call "MyWindow.exe"

del "MyWindow.obj"
del "MyWindow.exe"

EXIT (0)
