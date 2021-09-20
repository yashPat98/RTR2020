mkdir -p Window.app/Contents/MacOS
clang -o Window.app/Contents/MacOS/Window window.m -framework Cocoa
