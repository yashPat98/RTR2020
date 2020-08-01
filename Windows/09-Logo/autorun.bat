@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Logo.cpp"
rc.exe "Logo.rc"
link.exe "Logo.obj" "Logo.res" "user32.lib" "gdi32.lib"

call "Logo.exe"

del "Logo.obj"
del "Logo.res"
del "Logo.exe"

EXIT (0)
