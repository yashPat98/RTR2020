@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cl.exe /c /EHsc "DecoratedMsgBox.cpp"
link.exe "DecoratedMsgBox.obj" "user32.lib" "gdi32.lib"

call "DecoratedMsgBox.exe"

del "DecoratedMsgBox.obj"
del "DecoratedMsgBox.exe"

EXIT (0)
