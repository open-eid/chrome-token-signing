import sys

def get_exe():
    if sys.platform == 'darwin':
        return "host-osx/build/Release/chrome-token-signing.app/Contents/MacOS/chrome-token-signing"
    elif sys.platform == "linux2":
        return "host-linux/chrome-token-signing"
    elif sys.platform == 'win32':
        return "host-windows\\Debug\\host-windows.exe"
    else:
        print("Unsupported platform: %s" % sys.platform)
        sys.exit(1)

                                                        
