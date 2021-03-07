@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "WM_LBUTTONDOWN.cpp"
link.exe "WM_LBUTTONDOWN.obj" "user32.lib" "gdi32.lib"

call "WM_LBUTTONDOWN.exe"

del "WM_LBUTTONDOWN.obj"
del "WM_LBUTTONDOWN.exe"

EXIT (0)
