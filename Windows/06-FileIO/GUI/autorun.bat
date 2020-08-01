@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "FileIO.cpp"
link.exe "FileIO.obj" "user32.lib" "gdi32.lib"

call "FileIO.exe"

del "FileIO.obj"
del "FileIO.exe"

EXIT (0)
