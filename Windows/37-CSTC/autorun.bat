@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "CSTC.cpp"
rc.exe "RESOURCES.rc"
link.exe "CSTC.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "CSTC.exe"

del "CSTC.obj"
del "RESOURCES.res"
del "CSTC.exe"

EXIT (0)
