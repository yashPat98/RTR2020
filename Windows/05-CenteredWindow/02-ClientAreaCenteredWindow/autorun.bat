@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "ClientAreaCenteredWindow.cpp"
link.exe "ClientAreaCenteredWindow.obj" "user32.lib" "gdi32.lib"

call "ClientAreaCenteredWindow.exe"

del "ClientAreaCenteredWindow.obj"
del "ClientAreaCenteredWindow.exe"

EXIT (0)
