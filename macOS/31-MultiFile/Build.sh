mkdir -p Window.app/Contents/MacOS
clang -o Window.app/Contents/MacOS/Window main.m AppDelegate.m MyView.m -framework Cocoa
