@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "ScreenCenteredWindow.cpp"
link.exe "ScreenCenteredWindow.obj" "user32.lib" "gdi32.lib"

call "ScreenCenteredWindow.exe"

del "ScreenCenteredWindow.obj"
del "ScreenCenteredWindow.exe"

EXIT (0)
