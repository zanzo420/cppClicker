#include "autoclicker.h"
#include <QApplication>
#include <QTextStream>
#include <QDebug>
#include <iostream>
#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <winsock2.h>
#include <stdio.h>
#include <wininet.h>
#include <math.h>
#include <Windows.h>
#include <list>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <iterator>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <objidl.h>
#include <gdiplus.h>
#include <fstream>
#include <map>
#include <intrin.h>
#include <Iphlpapi.h>
#include <Mmsystem.h>
#include <Dbghelp.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "User32.lib")

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    AutoClicker w;
    w.show();

    return app.exec();
}
