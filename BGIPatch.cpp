// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"

vector<wstring>rawlist;
vector<wstring>newlist;

map<DWORD, wstring> SCRList;
map<DWORD, wstring> REPList;



char* RetAddr;

void _stdcall AymTextOutA(char* text)
{
    if (strlen(text) > 0 && text[0] != 0x00) 
    {
        DWORD len = strlen(text);
        char* txt = (char*)malloc(len * 2);
        memset(txt, 0, len * 2);
        if ((unsigned int)text[0] == 0x03)
            memcpy(txt, text + 1, len - 1);
        else
            memcpy(txt, text, len);

        CRC32 crc;
        wstring gbktext = ctowJIS(txt);
        DWORD strcrc = crc.Calc((char*)gbktext.c_str(), gbktext.length() * 2);
        auto scitr = REPList.find(strcrc);
        if (scitr != REPList.end())
        {
            char* tmp = wtoc((scitr->second).c_str());
            //lstrcpyA(Text, tmp);
            RetAddr = tmp;
        }
        else
        {
            char* mText = wtocUTF(gbktext.c_str());
            cout << "MISS:" << "0x" << hex << strcrc << "|" << wtoc(gbktext.c_str()) << endl;
            FILE* fp = fopen("MissText.txt", "ab+");
            fwrite("-------------------------------------------------------", 55, 1, fp);
            fwrite("\r\n", 2, 1, fp);
            fwrite(mText, strlen(mText), 1, fp);
            fwrite("\r\n", 2, 1, fp);
            fwrite("-------------------------------------------------------", 55, 1, fp);
            fwrite("\r\n", 2, 1, fp);
            fclose(fp);
        }
    }
}

PVOID pTextOutA = (PVOID)(BaseAddr + 0x32d66);
__declspec(naked)void mTextOutA()
{
    __asm
    {
        pushad
        pushfd
        push edi
        call AymTextOutA
        popfd
        popad
        cmp RetAddr, 0
        je have
        mov edi, RetAddr
        mov RetAddr, 0
    have:
        jmp pTextOutA
    }
}

void InitInlineHook()
{
    DetourTransactionBegin();
    DetourAttach((void**)&pTextOutA, mTextOutA);
    DetourTransactionCommit();
}


/// /////////////////////////////////////////////////////////
typedef LCID(WINAPI* fnGetUserDefaultLCID)(void);
typedef LANGID(WINAPI* fnGetSystemDefaultLangID)(void);
typedef LANGID(WINAPI* fnGetSystemDefaultUILanguage)(void);
typedef bool (WINAPI* fnSetWindowTextA)(HWND hWnd, LPCSTR lpString);
typedef HFONT(WINAPI* fnCreateFontA)(_In_ int     nHeight,
    _In_ int     nWidth,
    _In_ int     nEscapement,
    _In_ int     nOrientation,
    _In_ int     fnWeight,
    _In_ DWORD   fdwItalic,
    _In_ DWORD   fdwUnderline,
    _In_ DWORD   fdwStrikeOut,
    _In_ DWORD   fdwCharSet,
    _In_ DWORD   fdwOutputPrecision,
    _In_ DWORD   fdwClipPrecision,
    _In_ DWORD   fdwQuality,
    _In_ DWORD   fdwPitchAndFamily,
    _In_ LPCTSTR lpszFace);
typedef int (WINAPI* fnEnumFontFamiliesExA)(
    HDC           hdc,
    LPLOGFONTA    lpLogfont,
    FONTENUMPROCA lpProc,
    LPARAM        lParam,
    DWORD         dwFlags
    );
typedef int (WINAPI* fnMessageboxA)(
    _In_opt_ HWND    hWnd,
    _In_opt_ LPCSTR lpText,
    _In_opt_ LPCSTR lpCaption,
    _In_     UINT    uType
    );
PVOID g_pOldCreateFileA = CreateFileA;
typedef HANDLE(WINAPI* PfuncCreateFileA)(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );
fnMessageboxA MessageBoxAOLD;
fnCreateFontA pCreateFontA;
fnGetSystemDefaultLangID pGetSystemDefaultLangID;
fnGetSystemDefaultUILanguage pGetSystemDefaultUILanguage;
fnGetUserDefaultLCID pGetUserDefaultLCID;
fnSetWindowTextA pSetWindowTextA;
fnEnumFontFamiliesExA pEnumFontFamiliesExA;
typedef int(WINAPI* PfuncMultiByteToWideChar)(
    _In_      UINT   CodePage,
    _In_      DWORD  dwFlags,
    _In_      LPCSTR lpMultiByteStr,
    _In_      int    cbMultiByte,
    _Out_opt_ LPWSTR lpWideCharStr,
    _In_      int    cchWideChar);

HFONT WINAPI newCreateFontA(_In_ int     nHeight,
    _In_ int     nWidth,
    _In_ int     nEscapement,
    _In_ int     nOrientation,
    _In_ int     fnWeight,
    _In_ DWORD   fdwItalic,
    _In_ DWORD   fdwUnderline,
    _In_ DWORD   fdwStrikeOut,
    _In_ DWORD   fdwCharSet,
    _In_ DWORD   fdwOutputPrecision,
    _In_ DWORD   fdwClipPrecision,
    _In_ DWORD   fdwQuality,
    _In_ DWORD   fdwPitchAndFamily,
    _In_ LPCSTR lpszFace)
{
    fdwCharSet = GB2312_CHARSET;
    return CreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, L"黑体");
}

LCID WINAPI newGetUserDefaultLCID()
{
    return 0x411;
}

LANGID WINAPI newGetSystemDefaultLangID()
{
    return 0x411;
}

LANGID WINAPI newGetSystemDefaultUILanguage()
{
    return 0x411;
}

bool WINAPI newSetWindowTextA(HWND hw, LPCSTR lps)
{
    wchar_t newtitle[] = L"【白井木学园汉化组】11月的理想乡 - v1.0";
    return SetWindowTextW(hw, newtitle);
}

int WINAPI newEnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpProc, LPARAM lParam, DWORD dwFlags)
{
    LOGFONTA Lfont;
    memcpy(&Lfont, lpLogfont, sizeof(LOGFONTA));
    Lfont.lfCharSet = GB2312_CHARSET;
    memcpy(lpLogfont, &Lfont, sizeof(LOGFONTA));
    return ((fnEnumFontFamiliesExA)pEnumFontFamiliesExA)(hdc, lpLogfont, lpProc, lParam, dwFlags);
}

int WINAPI MessageBoxAEx(_In_opt_ HWND    hWnd,
    _In_opt_ LPCSTR lpText,
    _In_opt_ LPCSTR lpCaption,
    _In_     UINT    uType)
{
    char* Pstr = wtocGBK(ctowJIS((char*)lpText));
    char* Pstr2 = wtocGBK(ctowJIS((char*)lpCaption));
    return MessageBoxAOLD(hWnd, Pstr, Pstr2, uType);
}

std::wstring ReplaceCR(const wstring& orignStr)
{
    size_t pos = 0;
    wstring tempStr = orignStr;
    wstring oldStr = L"\\n";
    wstring newStr = L"\n";
    wstring::size_type newStrLen = newStr.length();
    wstring::size_type oldStrLen = oldStr.length();
    while (true)
    {
        pos = tempStr.find(oldStr, pos);
        if (pos == wstring::npos) break;

        tempStr.replace(pos, oldStrLen, newStr);
        pos += newStrLen;

    }

    return tempStr;
}

void InitText()
{
    ifstream fin("Script.txt");
    const int LineMax = 0x1000;
    char str[LineMax];
    if (fin.is_open())
    {
        int counter = 0;
        while (fin.getline(str, LineMax))
        {
            auto wtmp = ctowUTF(str);
            wstring wline = ReplaceCR(wtmp);
            if (wline[0] == (wchar_t)L'/' && wline[1] == (wchar_t)L'/')
            {
                wline = wline.substr(2);
                rawlist.push_back(wline);
            }
            else
            {
                newlist.push_back(wline);
            }
        }
        fin.close();
        size_t index = 0;
        CRC32 crc;
        for (index; index < rawlist.size(); ++index)
        {
            crc.Init();
            DWORD crcval = crc.Calc((char*)rawlist.at(index).c_str(), rawlist.at(index).length() * 2);
            REPList.insert(pair<DWORD, wstring>(crcval, newlist.at(index).c_str()));
            //cout << "0x" << hex << crcval << "|" << wtoc(newlist.at(index).c_str()) << endl;
        }
    }
    else
    {
        fin.close();
        MessageBox(0, L"Can't open script file", L"InitText Error", 0);
        return;
    }
}

void EdenInitHook()
{
    pCreateFontA = (fnCreateFontA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontA");
    pEnumFontFamiliesExA = (fnEnumFontFamiliesExA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "EnumFontFamiliesExA");
    pGetSystemDefaultLangID = (fnGetSystemDefaultLangID)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetSystemDefaultLangID");
    pGetSystemDefaultUILanguage = (fnGetSystemDefaultUILanguage)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetSystemDefaultUILanguage");
    pGetUserDefaultLCID = (fnGetUserDefaultLCID)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetUserDefaultLCID");
    pSetWindowTextA = (fnSetWindowTextA)GetProcAddress(GetModuleHandle(L"User32.dll"), "SetWindowTextA");
    MessageBoxAOLD = (fnMessageboxA)GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxA");
    DetourTransactionBegin();
    DetourAttach((void**)&pCreateFontA, newCreateFontA);
    DetourAttach((void**)&pGetSystemDefaultLangID, newGetSystemDefaultLangID);
    DetourAttach((void**)&pGetSystemDefaultUILanguage, newGetSystemDefaultUILanguage);
    DetourAttach((void**)&pGetUserDefaultLCID, newGetUserDefaultLCID);
    DetourAttach((void**)&pSetWindowTextA, newSetWindowTextA);
    DetourAttach((void**)&pEnumFontFamiliesExA, newEnumFontFamiliesExA);
    DetourAttach((void**)&MessageBoxAOLD, MessageBoxAEx);
    if (DetourTransactionCommit() != NO_ERROR)
    {
        MessageBoxW(NULL, L"HOOK ERROR!!!", L"AymKze", MB_OK | MB_ICONWARNING);
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        make_console();
        InitText();
        InitInlineHook();
        EdenInitHook();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

