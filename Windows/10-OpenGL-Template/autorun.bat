@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "OpenGL-Template.cpp"
rc.exe "RESOURCES.rc"
link.exe "OpenGL-Template.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "OpenGL-Template.exe"

del "OpenGL-Template.obj"
del "RESOURCES.res"
del "OpenGL-Template.exe"

EXIT (0)
