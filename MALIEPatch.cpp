// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"
map<unsigned int, wstring> TextList;
map<unsigned int, wstring> SelList;

wstring MalieProcessStr::ProcessStr(WCHAR* str_in, size_t sz)
{
    wstring str(str_in, sz);
    //size_t sz = str.size();
    bool fl_rub = 0, fl_vol = 0;
    wstring ret(sz + 3, 0);
    size_t len = 0;
    WORD pch;
    for (size_t idx = 0; idx < sz; idx++)
    {
        switch (str[idx])
        {
        case 0:
            if (fl_rub || fl_vol) fl_rub = fl_vol = 0;
            ret[len++] = EOSTR;
            break;
        case 1:
            idx += 4;
            break;
        case 2:
            ++idx;
            break;
        case 3:
            idx += 2;
            break;
        case 4:
            ++idx;
            break;
        case 5:
            idx += 2;
            break;
        case 6:
            ret[len++] = CH_HJN;
            pch = str[++idx];
            if (pch == 155) pch = 15;/*fix Mr.Masada's bug*/
            if (pch == 100) pch = 16;/*fix Mr.Masada's bug*/
            ret[len++] = Jindai2Kana[pch];
            break;
        case 7:
            switch (str[++idx])
            {
            case 0x0001://递归调用文字读取，然后继续处理（包含注释的文字）
                ret[len++] = TO_RUB;
                fl_rub = 1;
                break;
            case 0x0004://下一句自动出来
                ret[len++] = NXL;
                break;
            case 0x0006://代表本句结束
                ret[len++] = TO_RTN;
                break;
            case 0x0007://递归调用文字读取然后wcslen，跳过不处理。应该是用于注释
                ++idx;
                idx += wcslen(&str[idx]);
                break;
            case 0x0008://LoadVoice 后面是Voice名
                ret[len++] = TO_VOL;
                fl_vol = 1;
                break;
            case 0x0009://LoadVoice结束
                ret[len++] = EOVOL;
                break;
            default:
                ret[len++] = UNKNOW_SIG;
                break;
            }
            break;
        default:
            ret[len++] = str[idx];
        }
    }
    ret[len++] = EOPAR;
    return ret;
}

wstring MalieProcessStr::ProcessChsStr(WCHAR* str_in, unsigned int size)
{
    WORD v;
    static WORD list[0x005A] = 
    {
           0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0004,
           0x0005, 0x0005, 0x0006, 0x002F, 0x0007, 0x0030, 0x0008, 0x0031,
           0x0009, 0x0032, 0x000A, 0x0033, 0x000B, 0x0034, 0x000C, 0x0035,
           0x000D, 0x0036, 0x000E, 0x0037, 0x000F, 0x0038, 0x0010, 0x0039,
           0x0011, 0x003A, 0x0012, 0x0012, 0x003B, 0x0013, 0x003C, 0x0014,
           0x003D, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001A, 0x003E,
           0x0043, 0x001B, 0x003F, 0x0044, 0x001C, 0x0040, 0x0045, 0x001D,
           0x0041, 0x0046, 0x001E, 0x0042, 0x0047, 0x001F, 0x0020, 0x0021,
           0x0022, 0x0023, 0x0024, 0x0024, 0x0025, 0x0025, 0x0026, 0x0026,
           0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002C, 0x0302,
           0x0304, 0x002D, 0x002E, 0x0048, 0x0006, 0x0009, 0x4801, 0x4802,
           0x4804, 0x4805
           /*0x03xx>Wi,We,Wo 0x48xx>Vi,Ve,Vo*/
    };

    wstring chs = wstring(str_in, size);
    wstring ret(0x1000, 0);
    unsigned int len = 0;
    WORD ch;
    for (unsigned int idx = 0; idx < size; idx++)
    {
        switch (chs[idx])
        {
        case 0x000D:
        case EOPAR:
            break;
        case TO_VOL:
            ret[len++] = 0x0007; 
            ret[len++] = 0x0008; 
            ret_len += 2;
            break;
        case EOVOL:
            ret[len++] = 0x0007; 
            ret[len++] = 0x0009; 
            ret_len += 2;
            break;
        case STRUB:
            ret[len++] = 0x000A; 
            ret_len += 1;
            break;
        case EOSTR:
            ret[len++] = 0x0000; 
            ret_len += 1;
            break;
        case NXL:
            ret[len++] = 0x0007; 
            ret[len++] = 0x0004; 
            ret_len += 2;
            break;
        case TO_RTN:
            ret[len++] = 0x0007; 
            ret[len++] = 0x0006; 
            ret_len += 2;
            break;
        case TO_RUB:
            ret[len++] = 0x0007; 
            ret[len++] = 0x0001; 
            ret_len += 2;
            break;
        case CH_HJN:
            ch = chs[idx++];
            ch = chs[idx];
            if (0xFF1F == ch) 
            {
                ret[len++] = 0x0006;
                ret[len++] = 0x0000;
                ret_len += 2;
                break;
            }
            if (0xFF01 == ch) 
            {
                ret[len++] = 0x0006;
                ret[len++] = 0x0049;
                ret_len += 2;
                break;
            }
            if (0x3041 <= ch && ch <= 0x3096) 
                v = ch - 0x3041;/*Hiragana*/
            else if (0x30A1 <= ch && ch <= 0x30FA) 
                v = ch - 0x30A1;/*Katakana*/
            else 
            {
                ret[len++] = CH_HJN;/*as origin*/
                ret[len++] = ch;
                cout << "!!! WTF CHAR IS ORIGIN !!!" << endl;
                ret_len += 2;
                break;
            };/*Non-Kana*/
            if (list[v] & 0xFF00) 
            {
                ret[len++] = 0x0006; 
                ret[len++] = list[v] >> 8;
                ret[len++] = 0x0006; 
                ret[len++] = list[v] & 0x00FF;
                ret_len += 4;
            }/*Dual Kana*/
            else 
            {
                ret[len++] = 0x0006;
                ret[len++] = list[v];
                ret_len += 2;
            }
            break;
        default:
            ret[len++] = chs[idx];
            ret_len += 1;
        }
    }
    return move(ret);
}

WCHAR NormalTextBuffer[0x1000] = { 0 };
unsigned int NormalTextBufferOffset;
unsigned int NormalTextBufferSize;
void _stdcall HookPushStr(unsigned int VM_CODE, unsigned int offset, unsigned int size)
{
    auto scitr = TextList.find(offset);
    if (scitr != TextList.end())
    {
        MPS.ret_len = 0;
        memset(NormalTextBuffer, 0, 0x1000);

        wstring ret = MPS.ProcessChsStr((WCHAR*)scitr->second.c_str(), scitr->second.length());
        WCHAR* rep = (WCHAR*)ret.c_str();
        NormalTextBufferSize = MPS.ret_len * 2 + 2;//补全最后一位L"\0"
        memcpy(NormalTextBuffer, rep, NormalTextBufferSize);
        NormalTextBufferOffset = (unsigned int)NormalTextBuffer - VM_CODE;
        cout << "Offset:" << offset << "|Size:" << NormalTextBufferSize << "|str:" << wtoc(scitr->second.c_str()) << endl;
    }
    else
    {
        WCHAR ERR[0x1000];
        wsprintf(ERR, L"Miss text at:%d", offset);
        MessageBox(0, ERR, L"HookPushStr", MB_OK | MB_ICONERROR);
    }

}

/*
00525D40 | 83EC 08           | SUB     ESP, 8                         |
00525D43 | 53                | PUSH    EBX                            |
00525D44 | 8B5C24 10         | MOV     EBX, DWORD PTR SS:[ESP + 10]   |
00525D48 | 55                | PUSH    EBP                            |
00525D49 | 56                | PUSH    ESI                            |
00525D4A | 57                | PUSH    EDI                            |
00525D4B | 8B7C24 24         | MOV     EDI, DWORD PTR SS:[ESP + 24]   |
00525D4F | 66:C707 0000      | MOV     WORD PTR DS:[EDI], 0           |
00525D54 | 8B43 08           | MOV     EAX, DWORD PTR DS:[EBX + 8]    |
00525D57 | 8B48 20           | MOV     ECX, DWORD PTR DS:[EAX + 20]   |
00525D5A | 85C9              | TEST    ECX, ECX                       |
00525D5C | 0F8E 8D000000     | JLE     malie.525DEF                   |
00525D62 | 8B6C24 20         | MOV     EBP, DWORD PTR SS:[ESP + 20]   |
00525D66 | 3BE9              | CMP     EBP, ECX                       |
00525D68 | 0F8D AA000000     | JGE     malie.525E18                   |
00525D6E | 68 7CB95700       | PUSH    malie.57B97C                   | 57B97C:L"FILE"
00525D73 | 68 108A5700       | PUSH    <malie.sub_578A10>             | 578A10:L".\\system\\exec.dat"
......
00525DFA | 8B53 08           | MOV     EDX, DWORD PTR DS:[EBX + 8]    |
00525DFD | 8B48 04           | MOV     ECX, DWORD PTR DS:[EAX + 4]    |
00525E00 | 8B18              | MOV     EBX, DWORD PTR DS:[EAX]        |
00525E02 | 8BC1              | MOV     EAX, ECX                       |
00525E04 | 8B72 10           | MOV     ESI, DWORD PTR DS:[EDX + 10]   |
00525E07 | 83C4 08           | ADD     ESP, 8                         | Hook at here
*/

PVOID pHookPushStr = (PVOID)0x525E07;//(PVOID)(0x10021C4D);
__declspec(naked)void gHookPushStr()
{
    __asm
    {
        mov NormalTextBufferOffset, ebx;
        mov NormalTextBufferSize, ecx;
        pushad;
        pushfd;
        push ecx; // size
        push ebx; // offset
        push esi; // str
        call HookPushStr;
        popfd;
        popad;
        mov ebx, NormalTextBufferOffset;
        mov ecx, NormalTextBufferSize;
        jmp pHookPushStr;
    }
}
WCHAR SelectTextBuffer[0x1000];
unsigned int SelectTextBufferOffset;
void _stdcall HookPushStr2(unsigned int VM_DATA, unsigned int offset)
{
    WCHAR* str = (WCHAR*)(VM_DATA + offset);
    memset(SelectTextBuffer, 0, 0x1000);
    CRC32 crc;
    wstring gbktext(str);
    DWORD strcrc = crc.Calc((char*)gbktext.c_str(), gbktext.length() * 2);
    auto scitr = SelList.find(strcrc);
    if (scitr != SelList.end())
    {
        wcscpy(SelectTextBuffer, (scitr->second).c_str());
        SelectTextBufferOffset = (unsigned int)SelectTextBuffer - VM_DATA;
        cout << "offset:0x" << hex << offset;
        cout << "|" << wtocGBK((scitr->second).c_str()) << endl;
    }
}

/*
005326B7 | E8 C4080000       | CALL    <malie.sub_532F80>             |
005326BC | EB 0C             | JMP     malie.5326CA                   |
005326BE | E8 DD080000       | CALL    <malie.sub_532FA0>             |
005326C3 | EB 05             | JMP     malie.5326CA                   |
005326C5 | E8 96080000       | CALL    <malie.sub_532F60>             |
005326CA | 8B0D 70277400     | MOV     ECX, DWORD PTR DS:[742770]     | ecx:_NtUserGetMessage@16+C
005326D0 | 8B15 90277400     | MOV     EDX, DWORD PTR DS:[742790]     | 00742790:&L"■ [MALIE_LABEL] : %s\n"
005326D6 | 83E9 04           | SUB     ECX, 4                         | Hook at here
005326D9 | 03D0              | ADD     EDX, EAX                       |
005326DB | A1 8C277400       | MOV     EAX, DWORD PTR DS:[74278C]     | 0074278C:"`"
005326E0 | 890D 70277400     | MOV     DWORD PTR DS:[742770], ECX     | ecx:_NtUserGetMessage@16+C
005326E6 | 5F                | POP     EDI                            |
005326E7 | 5E                | POP     ESI                            |
005326E8 | 891401            | MOV     DWORD PTR DS:[ECX + EAX], EDX  | ecx+eax*1:_NtUserGetMessage@16+D
005326EB | B8 01000000       | MOV     EAX, 1                         |
005326F0 | C3                | RET                                    |
*/

PVOID pHookPushStr2 = (PVOID)0x5326D6;//(PVOID)(0x10021C4D);
__declspec(naked)void gHookPushStr2()
{
    __asm
    {
        mov SelectTextBufferOffset, eax;
        pushad;
        pushfd;
        push eax; // offset
        push edx; // str
        call HookPushStr2;
        popfd;
        popad;
        mov eax, SelectTextBufferOffset;
        jmp pHookPushStr2;
    }
}

void __stdcall GetSystemText(wchar_t* Text)
{
    if (Text[0] < 0x40)
        return;
    AymReplaceTextW(Text, (wchar_t*)L"終了します。\nよろしいですか？", (wchar_t*)L"即将结束游戏，\n确认吗？");
    //AymReplaceTextW(Text, (wchar_t*)L"No. 000 を上書きしますか？", (wchar_t*)L"即将覆盖No. 000 \n确认吗？");
    if (wcsstr(Text, L"を上書きしますか") != NULL)
        wcscpy(Text, L"即将覆盖存档 \n确认吗？");
    AymReplaceTextW(Text, (wchar_t*)L"コメントを入力して下さい。", (wchar_t*)L"请输入注释。");
    AymReplaceTextW(Text, (wchar_t*)L"記録しました。", (wchar_t*)L"存档完成。");
    AymReplaceTextW(Text, (wchar_t*)L"高速再開しますか？", (wchar_t*)L"要快速读档吗？");
    AymReplaceTextW(Text, (wchar_t*)L"次のシーンへ進みます。\nよろしいでしょうか？", (wchar_t*)L"即将跳转到下一事件，\n确认吗？");
    AymReplaceTextW(Text, (wchar_t*)L"クイックジャンプしますか？", (wchar_t*)L"要跳转到指定位置吗？");
    AymReplaceTextW(Text, (wchar_t*)L"開始画面に戻ります。\nよろしいですか？", (wchar_t*)L"即将返回标题，\n确认吗？");
    AymReplaceTextW(Text, (wchar_t*)L"自動再開の記録を初期化します。\nよろしいでしょうか？", (wchar_t*)L"即将重置自动存档数据，\n确认吗？");
    AymReplaceTextW(Text, (wchar_t*)L"高速再開の記録を初期化します。\nよろしいでしょうか？", (wchar_t*)L"即将重置快速存档数据，\n确认吗？");
}
/*
004021D0 | 8B4424 08         | MOV     EAX, DWORD PTR SS:[ESP + 8]    |
004021D4 | 8B4C24 04         | MOV     ECX, DWORD PTR SS:[ESP + 4]    | ecx:_NtUserGetMessage@16+C
004021D8 | 50                | PUSH    EAX                            |
004021D9 | 51                | PUSH    ECX                            | ecx:_NtUserGetMessage@16+C
004021DA | E8 81FDFFFF       | CALL    <malie.sub_401F60>             |
004021DF | 83C4 08           | ADD     ESP, 8                         |
004021E2 | 85C0              | TEST    EAX, EAX                       |
004021E4 | 74 11             | JE      malie.4021F7                   |
004021E6 | 8B5424 0C         | MOV     EDX, DWORD PTR SS:[ESP + C]    |
004021EA | 8B40 14           | MOV     EAX, DWORD PTR DS:[EAX + 14]   |
004021ED | 52                | PUSH    EDX                            |
004021EE | 50                | PUSH    EAX                            |
004021EF | E8 8C420300       | CALL    <malie.sub_436480>             | Hook at here
004021F4 | 83C4 08           | ADD     ESP, 8                         |
004021F7 | C3                | RET                                    |
*/
PVOID pGetSystemText = (PVOID)0x4021EF;
__declspec(naked)void gGetSystemText()
{
    __asm
    {
        pushad;
        pushfd;
        push edx;
        call GetSystemText;
        popfd;
        popad;
        jmp pGetSystemText;
    }
}

void __stdcall GetDatPatch(WCHAR* DatName)
{
    cout << wtoc(DatName) << endl;
}

PVOID pGetDatPatch = (PVOID)0x51EC1C;
__declspec(naked)void gGetDatPatch()
{
    __asm
    {
        pushad;
        pushfd;
        push eax;
        call GetDatPatch;
        popfd;
        popad;
        jmp pGetDatPatch;
    }
}

void Patch()
{
    DetourTransactionBegin();

    DetourAttach(&pHookPushStr, gHookPushStr);
    DetourAttach(&pHookPushStr2, gHookPushStr2);
    DetourAttach(&pGetSystemText, gGetSystemText);
    //DetourAttach(&pGetDatPatch, gGetDatPatch); // 弃用，这玩意儿没用
    if (DetourTransactionCommit() != NOERROR)
    {
        MessageBox(NULL, L"Patch hook error", L"Patch", MB_OK);
        ExitProcess(-1);
    }
}

bool IsOnce = false;
PVOID g_pOldSetWindowTextW = NULL;
typedef int (WINAPI* PfuncSetWindowTextW)(HWND hwnd, LPCTSTR lpString);
int WINAPI NewSetWindowTextW(HWND hwnd, LPWSTR lpString)
{
    if (!lstrcmp(lpString, L"恄欘恄埿恄妝 弻擵岝")) 
    {
        lstrcpy(lpString, L"神咒神威神乐 曙光  ——Eushully图书馆组||翻：某科学的超电磁炮 程序：空穜");
        if (!IsOnce)
        {
            IsOnce = true;
            //由于带壳，所以需要延迟注入
            Patch();
        }
    }
    return ((PfuncSetWindowTextW)g_pOldSetWindowTextW)(hwnd, lpString);
}

PVOID g_pOldCreateFontIndirectW = NULL;
typedef int (WINAPI* PfuncCreateFontIndirectW)(LOGFONTW* lplf);
int WINAPI NewCreateFontIndirectW(LOGFONTW* lplf)
{
    //这里由于是竖排显示，所以需要加@。横排不需要加
    lplf->lfCharSet = GB2312_CHARSET;
    wcscpy_s(lplf->lfFaceName, L"@SimHei");
    return ((PfuncCreateFontIndirectW)g_pOldCreateFontIndirectW)(lplf);
}

void Init()
{
    g_pOldSetWindowTextW = DetourFindFunction("USER32.dll", "SetWindowTextW");
    g_pOldCreateFontIndirectW = DetourFindFunction("GDI32.dll", "CreateFontIndirectW");
    DetourTransactionBegin();
    DetourAttach(&g_pOldSetWindowTextW, NewSetWindowTextW);
    DetourAttach(&g_pOldCreateFontIndirectW, NewCreateFontIndirectW);
    if (DetourTransactionCommit() != NOERROR)
    {
        MessageBox(NULL, L"Init hook error", L"Init", MB_OK | MB_ICONERROR);
        ExitProcess(-1);
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
        if (!InitText() || !InitSel())
        {
            MessageBox(0, L"Can't comp script.", L"DllMain", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }
        Init();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

__declspec(dllexport)void WINAPI AyamiKaze()
{
}
