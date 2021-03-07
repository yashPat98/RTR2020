@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "GetDC.cpp"
link.exe "GetDC.obj" "user32.lib" "gdi32.lib"

call "GetDC.exe"

del "GetDC.obj"
del "GetDC.exe"

EXIT (0)
