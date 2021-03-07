@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Smiley.cpp"
rc.exe "RESOURCES.rc"
link.exe "Smiley.obj" "RESOURCES.res" "user32.lib" "gdi32.lib" /SUBSYSTEM:WINDOWS

call "Smiley.exe"

del "Smiley.obj"
del "RESOURCES.res"
del "Smiley.exe"

EXIT (0)
