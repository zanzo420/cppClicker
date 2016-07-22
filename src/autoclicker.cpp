#include "autoclicker.h"
#include "ui_autoclicker.h"
#define _WIN32_WINNT 0x0500
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
#include <random>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QString>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "User32.lib")

struct CPPClickerData{
    bool isAutoClickerEnabled;
    bool isMouseDown;
    int maxCPS;
    int minCPS;
    bool autoClickAlgorithm; //false = max random, true = legit looking
    bool clickType; //false = left, true = right
    bool isDoubleClickerEnabled;
    bool doubleClickEvent;
    int doubleClickerTime;
    bool isWTapEnabled;
    int wtapTime;
    int wtapSleepTime;
    bool wtapEvent;
};

int random(int min, int max);
void ReadJSONFile();
void WriteJSONFile();

//Port from .net, lol
struct IntBox{
    int max;
    int min;
    int use_count;
};

class IntBoxAlgorithm{
public:
    IntBox generateIntBox(int min, int max, int userRange);
    int generateIntFromBox(IntBox box);
};

int IntBoxAlgorithm::generateIntFromBox(IntBox box){
    box.use_count++;
    return random(box.min, box.max);
}

IntBox IntBoxAlgorithm::generateIntBox(int min, int max, int userRange){
    if (min > max || min > max - userRange)
    {
        IntBox box2;
        box2.min = 0xFFF;
        box2.max = 0xFFF;
        box2.use_count = 0xFFF;
        return box2;
    }
    IntBox box;
    box.min = random(min, max - userRange);
    box.max = box.min + userRange;
    box.use_count = 0;
    return box;
}

CPPClickerData autoclicker_data;
void AutoClickLoop();

bool IsWindowActive(){
    HWND hwnd = FindWindow(TEXT("cppClicker v1.0.0 by 3dsboy08"), 0);
    if (hwnd == GetActiveWindow())
    {
        return true;
    }else return false;
}

bool collectingKeyBind = false;
DWORD keyBind = 0x58; //key X

HHOOK MouseHook;
HHOOK hhkLowLevelKybd;
bool SupressMouseHook = false;

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam){
    BOOL eatMouseStroke2 = false;
    if(!SupressMouseHook){
        BOOL eatMouseStroke = false;
        if(autoclicker_data.isDoubleClickerEnabled){
            if(wParam == WM_LBUTTONUP){
                autoclicker_data.doubleClickEvent = true;
            }
        }else if(autoclicker_data.isWTapEnabled){
            if(wParam == WM_LBUTTONUP){
                autoclicker_data.wtapEvent = true;
            }
        }else if(autoclicker_data.isAutoClickerEnabled){
            if(wParam == WM_LBUTTONDOWN){
                autoclicker_data.isMouseDown = true;
            }else if(wParam == WM_LBUTTONUP){
                autoclicker_data.isMouseDown = false;
            }
        }
        return(eatMouseStroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
    }else{
        BOOL eatMouseStroke = false;
        return(eatMouseStroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
    }
    return(eatMouseStroke2 ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    BOOL fEatKeystroke = FALSE;

    if (nCode == HC_ACTION)
    {
        switch (wParam)
        {
        case WM_KEYDOWN:
            if(collectingKeyBind){
               fEatKeystroke = true;
               PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
               keyBind = p->vkCode;
               collectingKeyBind = false;
               MessageBox(NULL, L"Set keybind successfully!", L"cppClicker", MB_OK);
               return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
            }else{
                PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
                if(fEatKeystroke = (p->vkCode == keyBind)){
                    if(autoclicker_data.isAutoClickerEnabled){
                        autoclicker_data.isAutoClickerEnabled = false;
                    }else{
                        autoclicker_data.isAutoClickerEnabled = true;
                    }
                }
            }
            break;
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            break;
        }
    }
    return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

AutoClicker::AutoClicker(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutoClicker)
{
    ui->setupUi(this);
    ui->msBox->setVisible(false);
    ui->msLabel->setVisible(false);
    ui->waitUntilClick->setVisible(false);
    ui->wtapValueBox->setVisible(false);
    ui->wtapLabel->setVisible(false);
    ui->wtapMsLabel->setVisible(false);
    ui->wtapLabel_2->setVisible(false);
    ui->wtapMsLabel_2->setVisible(false);
    ui->wtapValueBox_2->setVisible(false);
    QFile file;
    file.setFileName("settings.json");
    if(!file.exists()){
        file.close();
        autoclicker_data.autoClickAlgorithm = true;
        autoclicker_data.clickType = false;
        autoclicker_data.doubleClickerTime = 100;
        autoclicker_data.isAutoClickerEnabled = false;
        autoclicker_data.isMouseDown = false;
        autoclicker_data.isDoubleClickerEnabled = false;
        autoclicker_data.doubleClickEvent = false;
        autoclicker_data.maxCPS = 10;
        autoclicker_data.minCPS = 5;
        autoclicker_data.isWTapEnabled = false;
        autoclicker_data.wtapTime = 100;
        autoclicker_data.wtapEvent = false;
        autoclicker_data.wtapSleepTime = 100;
        WriteJSONFile();
    }else{
        file.close();
        ReadJSONFile();
        autoclicker_data.isAutoClickerEnabled = false;
        autoclicker_data.isMouseDown = false;
        autoclicker_data.isDoubleClickerEnabled = false;
        autoclicker_data.isWTapEnabled = false;
        autoclicker_data.wtapEvent = false;
        autoclicker_data.doubleClickEvent = false;
        if(autoclicker_data.autoClickAlgorithm){
            ui->legitLookingBox->setChecked(true);
        }else{
            ui->legitLookingBox->setChecked(false);
            ui->maxRandomBox->setChecked(true);
        }
        if(autoclicker_data.autoClickAlgorithm){
            int index = ui->comboBox->findText("Left Mouse Button");
            ui->comboBox->setCurrentIndex(index);
        }else{
            int index = ui->comboBox->findText("Right Mouse Button");
            ui->comboBox->setCurrentIndex(index);
        }
        ui->msBox->setValue(autoclicker_data.doubleClickerTime);
        ui->maxCPSBox->setValue(autoclicker_data.maxCPS);
        ui->minCPSBox->setValue(autoclicker_data.minCPS);
        ui->wtapValueBox->setValue(autoclicker_data.wtapTime);
        ui->wtapValueBox_2->setValue(autoclicker_data.wtapSleepTime);
    }
    MouseHook = SetWindowsHookEx(WH_MOUSE_LL,MouseHookProc, GetModuleHandle(NULL), 0);
    hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)AutoClickLoop, NULL, NULL, NULL);
}

IntBoxAlgorithm algorithm;
IntBox box;
int IntBoxIterations = 0;
void AutoClickLoop(){
    do{
        if(autoclicker_data.isAutoClickerEnabled && autoclicker_data.isMouseDown){
            if(autoclicker_data.autoClickAlgorithm){
                if (IntBoxIterations == 0)
                {
                    try{
                        box = algorithm.generateIntBox(autoclicker_data.minCPS, autoclicker_data.maxCPS, 3);
                        IntBoxIterations++;
                    }catch(std::exception exe){
                        exit(0);
                    }
                }
                else
                {
                    IntBoxIterations++;
                    if (IntBoxIterations == 20) IntBoxIterations = 0;
                }
                int SleepTime = 1000 / algorithm.generateIntFromBox(box);
                Sleep(SleepTime);
                if(autoclicker_data.clickType){
                    SupressMouseHook = true;
                    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                    Sleep(25);
                    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
                    SupressMouseHook = false;
                }else{
                    SupressMouseHook = true;
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    Sleep(25);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    SupressMouseHook = false;
                }
            }else{
                int prenum = random(autoclicker_data.minCPS, autoclicker_data.maxCPS);
                int rand = random(0, 1);
                bool addSub;
                if(rand){
                    addSub = true;
                }else{
                    addSub = false;
                }
                if (addSub)
                {
                    prenum += random(0, 3);
                }
                else
                {
                    prenum -= random(0, 3);
                }
                int SleepTime = 1000 / prenum;
                Sleep(SleepTime);
                if(autoclicker_data.clickType){
                    SupressMouseHook = true;
                    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                    Sleep(25);
                    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
                    SupressMouseHook = false;
                }else{
                    SupressMouseHook = true;
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    Sleep(25);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    SupressMouseHook = false;
                }
            }
        }else if(autoclicker_data.doubleClickEvent){
            autoclicker_data.doubleClickEvent = false;
            Sleep(autoclicker_data.doubleClickerTime);
            SupressMouseHook = true;
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            Sleep(25);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            SupressMouseHook = false;
        }else if(autoclicker_data.wtapEvent){
            autoclicker_data.wtapEvent = false;
            Sleep(autoclicker_data.wtapTime);
            keybd_event(0x57, 0x14, 0, 0); //0x57 = w
            Sleep(autoclicker_data.wtapSleepTime);
            keybd_event(0x57, 0x94, KEYEVENTF_KEYUP, 0);
        }
        Sleep(1);
    }while(true);
}

std::random_device rd; // obtain a random number from hardware
std::mt19937 eng(rd()); // seed the generator
int random(int min, int max) //range : [min, max)
{
    std::uniform_int_distribution<> distr(min, max); // define the range
    return distr(eng);
}


AutoClicker::~AutoClicker()
{
    delete ui;
}

void AutoClicker::on_actionExit_2_triggered()
{
    exit(0);
}

void AutoClicker::on_doubleClickBox_clicked()
{
    if(autoclicker_data.isAutoClickerEnabled || autoclicker_data.isWTapEnabled){
        MessageBox(NULL, L"ERROR: You can only enable 1 module at a time.", L"cppClicker ERROR", MB_OK);
        ui->doubleClickBox->setChecked(false);
    }
    if(this->ui->doubleClickBox->isChecked()){
        ui->msBox->setVisible(true);
        ui->msLabel->setVisible(true);
        ui->waitUntilClick->setVisible(true);
        autoclicker_data.isDoubleClickerEnabled = true;
    }else{
        ui->msBox->setVisible(false);
        ui->msLabel->setVisible(false);
        ui->waitUntilClick->setVisible(false);
        autoclicker_data.isDoubleClickerEnabled = false;
    }
}

void AutoClicker::on_minCPSBox_valueChanged(int arg1)
{
    autoclicker_data.minCPS = arg1;
}

void AutoClicker::on_maxCPSBox_valueChanged(int arg1)
{
    autoclicker_data.maxCPS = arg1;
}

void AutoClicker::on_maxRandomBox_clicked()
{
    autoclicker_data.autoClickAlgorithm = false;
}

void AutoClicker::on_legitLookingBox_clicked()
{
    autoclicker_data.autoClickAlgorithm = true;
}

void AutoClicker::on_comboBox_currentIndexChanged(const QString &arg1)
{
    if(arg1.toStdString().c_str() == "Left Mouse Button"){
        autoclicker_data.clickType = false;
    }else{
        autoclicker_data.clickType = true;
    }
}

void AutoClicker::on_pushButton_clicked()
{
    collectingKeyBind = true;
}

void AutoClicker::on_msBox_valueChanged(int arg1)
{
    autoclicker_data.doubleClickerTime = arg1;
}

void AutoClicker::on_wtapBox_clicked()
{
    if(autoclicker_data.isAutoClickerEnabled || autoclicker_data.isDoubleClickerEnabled){
        MessageBox(NULL, L"ERROR: You can only enable 1 module at a time.", L"cppClicker ERROR", MB_OK);
        ui->wtapBox->setChecked(false);
    }
    if(this->ui->wtapBox->isChecked()){
        ui->wtapValueBox->setVisible(true);
        ui->wtapLabel->setVisible(true);
        ui->wtapMsLabel->setVisible(true);
        ui->wtapLabel_2->setVisible(true);
        ui->wtapMsLabel_2->setVisible(true);
        ui->wtapValueBox_2->setVisible(true);
        autoclicker_data.isWTapEnabled = true;
    }else{
        ui->wtapValueBox->setVisible(false);
        ui->wtapLabel->setVisible(false);
        ui->wtapMsLabel->setVisible(false);
        ui->wtapLabel_2->setVisible(false);
        ui->wtapMsLabel_2->setVisible(false);
        ui->wtapValueBox_2->setVisible(false);
        autoclicker_data.isWTapEnabled = false;
    }
}

void AutoClicker::on_wtapValueBox_valueChanged(int arg1)
{
     autoclicker_data.wtapTime = arg1;
}

void AutoClicker::on_wtapValueBox_2_valueChanged(int arg1)
{
    autoclicker_data.wtapSleepTime = arg1;
}

void write_json(QJsonObject &json)
{
    json["maxcps"] = autoclicker_data.maxCPS;
    json["mincps"] = autoclicker_data.minCPS;
    json["autoclickalgorithm"] = autoclicker_data.autoClickAlgorithm;
    json["clicktype"] = autoclicker_data.clickType;
    json["doubleclickertime"] = autoclicker_data.doubleClickerTime;
    json["wtaptime"] = autoclicker_data.wtapTime;
    json["wtapsleeptime"] = autoclicker_data.wtapSleepTime;
}

void read_json(const QJsonObject &json)
{
    autoclicker_data.maxCPS = json["maxcps"].toInt(10);
    autoclicker_data.minCPS = json["mincps"].toInt(5);
    autoclicker_data.autoClickAlgorithm = json["autoclickalgorithm"].toBool(true);
    autoclicker_data.clickType = json["clicktype"].toBool(false);
    autoclicker_data.doubleClickerTime = json["doubleclickertime"].toInt(100);
    autoclicker_data.wtapTime = json["wtaptime"].toInt(100);
    autoclicker_data.wtapSleepTime = json["wtapsleeptime"].toInt(100);
}

void ReadJSONFile(){
    QFile file;
    QString value;
    file.setFileName("settings.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    value = file.readAll();
    file.close();
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8());
    QJsonObject object = document.object();
    read_json(object);
    return;
}

void WriteJSONFile(){
    QFile file;
    file.setFileName("settings.json");
    if(file.exists()) file.remove();
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QJsonObject object;
    write_json(object);
    QJsonDocument document = QJsonDocument(object);
    file.write(document.toJson());
    file.close();
}

void AutoClicker::on_actionSave_triggered()
{
    WriteJSONFile();
    MessageBox(NULL, L"Successfully saved settings!", L"cppClicker", MB_OK);
}
