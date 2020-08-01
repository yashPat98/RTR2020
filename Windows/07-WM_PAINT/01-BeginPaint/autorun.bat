@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "BeginPaint.cpp"
link.exe "BeginPaint.obj" "user32.lib" "gdi32.lib"

call "BeginPaint.exe"

del "BeginPaint.obj"
del "BeginPaint.exe"

EXIT (0)
