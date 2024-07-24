#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <vector>
#include <chrono>
#include <unordered_map>

HHOOK hhk = NULL;
std::unordered_map<DWORD, DWORD> obfMap, deobfMap;
const int PERIOD = 10;

void initKeyMaps() {
    for (DWORD i = 0; i < 256; ++i) {
        DWORD obfKey = (i + 1) % 256;
        obfMap[i] = obfKey;
        deobfMap[obfKey] = i;
    }
}

void updKeyMaps() {
    std::unordered_map<DWORD, DWORD> newObfMap, newDeobfMap;
    for (DWORD i = 0; i < 256; ++i) {
        DWORD obfKey = (i + rand()) % 256;
        newObfMap[i] = obfKey;
        newDeobfMap[obfKey] = i;
    }
    obfMap = newObfMap;
    deobfMap = newDeobfMap;
}

void sndRndKeys() {
    const std::string chrs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+[]{}|;:',.<>?/~` ";
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200 + (rand() % 800)));
        int len = rand() % 81 + 20;
        for (int i = 0; i < len; ++i) {
            char c = chrs[rand() % chrs.size()];
            bool shft = (rand() % 100) < 20;
            if (shft) {
                INPUT shftDn = {INPUT_KEYBOARD};
                shftDn.ki.wVk = VK_SHIFT;
                shftDn.ki.dwExtraInfo = 0x1234;
                SendInput(1, &shftDn, sizeof(INPUT));
            }
            INPUT in = {INPUT_KEYBOARD};
            in.ki.wVk = VkKeyScan(c) & 0xFF;
            in.ki.dwExtraInfo = 0x1234; //ignore
            SendInput(1, &in, sizeof(INPUT));
            in.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &in, sizeof(INPUT));
            if (shft) {
                INPUT shftUp = {INPUT_KEYBOARD};
                shftUp.ki.wVk = VK_SHIFT;
                shftUp.ki.dwFlags = KEYEVENTF_KEYUP;
                shftUp.ki.dwExtraInfo = 0x1234;
                SendInput(1, &shftUp, sizeof(INPUT));
            }
            DWORD obfVkCode = obfMap[VkKeyScan(c) & 0xFF];
            DWORD deobfVkCode = deobfMap[obfVkCode];
            in.ki.wVk = deobfVkCode;
            in.ki.dwFlags = 0;
            in.ki.dwExtraInfo = 0x1234; //ignore
            SendInput(1, &in, sizeof(INPUT));
            in.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &in, sizeof(INPUT));
        }
    }
}

LRESULT CALLBACK llKbPr(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKb = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN || wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            DWORD vkCode = pKb->vkCode;
            if (pKb->dwExtraInfo == 0x1234) return 1;
            if (vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) return CallNextHookEx(hhk, nCode, wParam, lParam);
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                DWORD obfVkCode = obfMap[vkCode];
                DWORD deobfVkCode = deobfMap[obfVkCode];
                INPUT in = {INPUT_KEYBOARD};
                in.ki.wVk = deobfVkCode;
                in.ki.dwFlags = 0;
                in.ki.dwExtraInfo = 0;
                SendInput(1, &in, sizeof(INPUT));
                in.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &in, sizeof(INPUT));
                return 1;
            }
        }
    }
    return CallNextHookEx(hhk, nCode, wParam, lParam);
}

void setHk() {
    hhk = SetWindowsHookEx(WH_KEYBOARD_LL, llKbPr, NULL, 0);
    if (hhk == NULL) std::cerr << "Failed to init!" << std::endl;
}

void clrConsole() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = {0, 0};
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten)) return;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten)) return;
    SetConsoleCursorPosition(hConsole, coordScreen);
}

void relaunchProg() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        char szFileName[MAX_PATH];
        GetModuleFileNameA(NULL, szFileName, MAX_PATH);
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if (CreateProcessA(szFileName, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            exit(0);
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    initKeyMaps();
    std::thread(sndRndKeys).detach();
    std::thread(relaunchProg).detach();
    setHk();
    clrConsole(); //cls
    std::cout << "You are now protected from keyloggers." << std::endl;
    MSG msg;
    time_t startTime = time(NULL);
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (difftime(time(NULL), startTime) >= PERIOD) {
            updKeyMaps();
            startTime = time(NULL);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
