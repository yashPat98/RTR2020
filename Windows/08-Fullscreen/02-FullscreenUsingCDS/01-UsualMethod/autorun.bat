@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "FullscreenUsingCDS.cpp"
link.exe "FullscreenUsingCDS.obj" "user32.lib" "gdi32.lib"

call "FullscreenUsingCDS.exe"

del "FullscreenUsingCDS.obj"
del "FullscreenUsingCDS.exe"

EXIT (0)
