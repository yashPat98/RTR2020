@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Kundli.cpp"
rc.exe "RESOURCES.rc"
link.exe "Kundli.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Kundli.exe"

del "Kundli.obj"
del "RESOURCES.res"
del "Kundli.exe"

EXIT (0)
