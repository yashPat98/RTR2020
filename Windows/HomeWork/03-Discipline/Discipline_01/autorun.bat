@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Discipline_01.cpp"
rc.exe "RESOURCES.rc"
link.exe "Discipline_01.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Discipline_01.exe"

del "Discipline_01.obj"
del "RESOURCES.res"
del "Discipline_01.exe"

EXIT (0)
