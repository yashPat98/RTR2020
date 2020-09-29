@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "3DCube.cpp"
rc.exe "RESOURCES.rc"
link.exe "3DCube.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "3DCube.exe"

del "3DCube.obj"
del "RESOURCES.res"
del "3DCube.exe"

EXIT (0)
