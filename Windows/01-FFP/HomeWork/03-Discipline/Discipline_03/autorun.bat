@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "Discipline_03.cpp"
rc.exe "RESOURCES.rc"
link.exe "Discipline_03.obj" "RESOURCES.res" "user32.lib" "gdi32.lib"

call "Discipline_03.exe"

del "Discipline_03.obj"
del "RESOURCES.res"
del "Discipline_03.exe"

EXIT (0)
