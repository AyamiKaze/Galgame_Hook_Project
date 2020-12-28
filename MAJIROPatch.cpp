// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "detours.h"
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <mutex>
extern "C" {
#include "xxhash.h"
}
#pragma comment(lib, "detours.lib")
using namespace std;
typedef HFONT(WINAPI* fnCreateFontA)(
	int nHeight, // logical height of font height
	int nWidth, // logical average character width
	int nEscapement, // angle of escapement
	int nOrientation, // base-line orientation angle
	int fnWeight, // font weight
	DWORD fdwItalic, // italic attribute flag
	DWORD fdwUnderline, // underline attribute flag
	DWORD fdwStrikeOut, // strikeout attribute flag
	DWORD fdwCharSet, // character set identifier
	DWORD fdwOutputPrecision, // output precision
	DWORD fdwClipPrecision, // clipping precision
	DWORD fdwQuality, // output quality
	DWORD fdwPitchAndFamily, // pitch and family
	LPCSTR lpszFace // pointer to typeface name string
	);
fnCreateFontA CreateFontAOLD;
HFONT WINAPI CreateFontAEx(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
	fdwCharSet = GB2312_CHARSET;
	//cout << lpszFace << endl;
	//return CreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, wcs);
	return CreateFontA(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, "黑体");
}

PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND(WINAPI* pfuncCreateWindowExA)(
	DWORD dwExStyle,
	LPCTSTR lpClassName,
	LPCTSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam);
HWND WINAPI NewCreateWindowExA(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	const char* szWndName = "琥珀结晶 | 简体中文化补丁V1.1";

	HWND hw = ((pfuncCreateWindowExA)g_pOldCreateWindowExA)(dwExStyle, lpClassName, (LPCTSTR)szWndName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	//SetWindowDisplayAffinity(hw, WDA_MONITOR);
	return hw;
}

PVOID g_pOldCreateFontIndirectA = CreateFontIndirectA;
typedef HFONT(WINAPI* PfuncCreateFontIndirectA)(LOGFONTA* lplf);
HFONT WINAPI NewCreateFontIndirectA(LOGFONTA* lplf)
{
	lplf->lfCharSet = GB2312_CHARSET;
	//cout << lplf->lfFaceName << endl;
	//if (!strcmp(lplf->lfFaceName, "MS UI Gothic"))
	//	strcpy(lplf->lfFaceName, "黑体");
	strcpy(lplf->lfFaceName, "黑体");
	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

typedef int (WINAPI* fnEnumFontFamiliesExA)(
	HDC           hdc,
	LPLOGFONTA    lpLogfont,
	FONTENUMPROCA lpProc,
	LPARAM        lParam,
	DWORD         dwFlags
	);
fnEnumFontFamiliesExA pEnumFontFamiliesExA;
int WINAPI newEnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpProc, LPARAM lParam, DWORD dwFlags)
{
	lpLogfont->lfCharSet = GB2312_CHARSET;
	LOGFONTA Lfont;
	memcpy(&Lfont, lpLogfont, sizeof(LOGFONTA));
	Lfont.lfCharSet = GB2312_CHARSET;
	memcpy(lpLogfont, &Lfont, sizeof(LOGFONTA));
	return ((fnEnumFontFamiliesExA)pEnumFontFamiliesExA)(hdc, lpLogfont, lpProc, lParam, dwFlags);
}


wchar_t szTitle[] = L"提示";
typedef int (WINAPI* fnMessageboxA)(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType
	);
fnMessageboxA MessageBoxAOLD;
int WINAPI MessageBoxAEx(_In_opt_ HWND    hWnd,
	_In_opt_ LPCSTR lpText,
	_In_opt_ LPCSTR lpCaption,
	_In_     UINT    uType)
{
	char* Pstr = wtocGBK(ctowJIS((char*)lpText));
	//cout << lpText << endl;
	//TXT << Pstr << endl;
	if (!strcmp(Pstr, "終了してよろしいですか？"))
	{
		strcpy(Pstr, "即将结束游戏，确认吗？");
	}
	else if (!strcmp(Pstr, "タイトルに戻ります。よろしいですか？"))
	{
		strcpy(Pstr, "即将返回标题，确认吗？");
	}
	return MessageBoxAOLD(hWnd, Pstr, wtocGBK(szTitle), uType);
}


void Hook()
{
	/*
	BYTE pat[] = { 0x86 };
	BYTE qute[] = { 0xB8, 0xA1 };
	BYTE qute2[] = { 0xB9, 0xA1 };

	BYTE save[] = { 0x5F, 0x63, 0x6E, 0x00 };

	memcopy((void*)(BaseAddr + 0x390eb), pat, sizeof(pat));

	memcopy((void*)(BaseAddr + 0x1894B), qute, sizeof(qute));
	memcopy((void*)(BaseAddr + 0x1896D), qute2, sizeof(qute2));
	memcopy((void*)(BaseAddr + 0x18997), qute, sizeof(qute));

	memcopy((void*)(BaseAddr + 0x193B1), qute, sizeof(qute));
	memcopy((void*)(BaseAddr + 0x19427), qute2, sizeof(qute2));
	//memcopy((void*)(BaseAddr + 0x88303), save, sizeof(save));

	BYTE qute3[] = { 0xA1, 0xBB, 0xA1, 0xB9, 0x00, 0x00, 0x00, 0x00, 0xA3, 0xA9, 0xA1, 0xB9 };
	BYTE qute4[] = { 0xA1, 0xB8, 0xA1, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xA1, 0xB8, 0xA3, 0xA8 };
	memcopy((void*)(BaseAddr + 0x88FD0), qute3, sizeof(qute3));
	memcopy((void*)(BaseAddr + 0x88FE0), qute4, sizeof(qute4));
*/
	CreateFontAOLD = (fnCreateFontA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontA");
	pEnumFontFamiliesExA = (fnEnumFontFamiliesExA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "EnumFontFamiliesExA");
	MessageBoxAOLD = (fnMessageboxA)GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxA");
	DetourTransactionBegin();
	DetourAttach((void**)& CreateFontAOLD, CreateFontAEx);
	DetourAttach(&g_pOldCreateFontIndirectA, NewCreateFontIndirectA);
	DetourAttach(&g_pOldCreateWindowExA, NewCreateWindowExA);
	DetourAttach((void**)& pEnumFontFamiliesExA, newEnumFontFamiliesExA);
	DetourAttach((void**)&MessageBoxAOLD, MessageBoxAEx);
	if (DetourTransactionCommit() != NOERROR) 
		MessageBox(NULL, L"ERROR", L"Aym", MB_OK);
}


int CharList[26][2] = {
	{0x60,0x81},
	{0x61,0x82},
	{0x62,0x83},
	{0x63,0x84},
	{0x64,0x85},
	{0x65,0x86},
	{0x66,0x87},
	{0x67,0x88},
	{0x68,0x89},
	{0x69,0x8a},
	{0x6a,0x8b},
	{0x6b,0x8c},
	{0x6c,0x8d},
	{0x6d,0x8e},
	{0x6e,0x8f},
	{0x6f,0x90},
	{0x70,0x91},
	{0x71,0x92},
	{0x72,0x93},
	{0x73,0x94},
	{0x74,0x95},
	{0x75,0x96},
	{0x76,0x97},
	{0x77,0x98},
	{0x78,0x99},
	{0x79,0x9a},
};

int (*sub_446DF0)(char* a1, LPCSTR lpFileName);
int MyFileHook(char* a1, LPCSTR lpFileName)
{
	if (!strcmp(a1, "update12"))
		strcpy(a1, "Amber_Quartz_CN");
	return sub_446DF0(a1, lpFileName);
}

void FileNameToLower(std::string& FileName)
{
	std::transform(FileName.begin(), FileName.end(), FileName.begin(), ::tolower);
}

char* (*sub_40B000)(char* a1);
char* MyMjoSuppost(char* a1)
{
	//cout << a1 << endl;
	//if (!strcmp(a1, "01僾儘儘乕僌俙慜曇_01"))
	//	return sub_40B000((char*)"01プロローグａ前編_01");
	for (int i = 0; i < 26; i++)
	{
		for (int a = 0; a < strlen(a1); a++)
		{
			if ((unsigned char)a1[a] == 0x82 && a1[a + 1] == CharList[i][0])
				a1[a + 1] = (char)CharList[i][1];
		}
	}
	string f(a1);
	//FileNameToLower(f);
	//unsigned long long HashFileName = XXH64(f.c_str(), strlen(f.c_str()), gen_toc_seed(f.c_str()));
	//return sub_40B000(wtocGBK(ctowJIS(a1)));
	//cout << (char*)(to_string(HashFileName).c_str()) << endl;
	//return sub_40B000((char*)(to_string(HashFileName).c_str()));
	return sub_40B000(a1);
}

bool IsSjis(char* text, int len)
{
	for (int i = 0; i < len; i++)
	{
		if ((unsigned char)text[i] > 0x7F)
			return true;
	}
	return false;
}

void __stdcall AymGetText(char* Text)
{
	if (strlen(Text) == 0)
		return;
	if (IsSjis(Text, strlen(Text))) 
	{
		char* text;
		text = wtocGBK(ctowJIS(Text));
		cs(text, "：", Text, "：");
		cs(text, "＠", Text, "＠");
		cs(text, "（", Text, "（");
		cs(text, "／", Text, "／");
		cs(text, "[", Text, "[");
		cs(text, "]", Text, "]");
		cs(text, "［", Text, "［");
		cs(text, "］", Text, "］");
		cs(text, "【", Text, "【");
		cs(text, "】", Text, "】");
		cs(text, "昴", Text, "昴");
		cs(text, "なゆた", Text, "那由他");
		cs(text, "智", Text, "智");
		cs(text, "弥夜子", Text, "弥夜子");
		cs(text, "サリナ", Text, "纱理奈");
		cs(text, "恭平", Text, "恭平");
		cs(text, "珠乃", Text, "珠乃");
		cs(text, "玉藻", Text, "玉藻");
		cs(text, "琴理", Text, "琴理");
		cs(text, "恒河", Text, "恒河");
		cs(text, "スバル", Text, "昴");
		cs(text, "子主", Text, "子主");
		cs(text, "コト", Text, "琴");
		cs(text, "キョウ", Text, "恭");
		cs(text, "ヒメ", Text, "姬");
		cs(text, "ミコ姉", Text, "弥子姐");
		cs(text, "トモ", Text, "智");
		cs(text, "＆", Text, "＆");
		cs(text, "白色", Text, "白色");
		cs(text, "主人公", Text, "主人公");
		cs(text, "昴", Text, "昴");
		cs(text, "スバル", Text, "昴");
		cs(text, "なゆた", Text, "那由他");
		cs(text, "智", Text, "智");
		cs(text, "トモ", Text, "智");
		cs(text, "サリナ", Text, "纱理奈");
		cs(text, "ヒメ", Text, "姬");
		cs(text, "弥夜子", Text, "弥夜子");
		cs(text, "ミコ姉", Text, "弥子姐");
		cs(text, "恭平", Text, "恭平");
		cs(text, "キョウ", Text, "恭");
		cs(text, "珠乃", Text, "珠乃");
		cs(text, "玉藻", Text, "玉藻");
		cs(text, "コト", Text, "琴");
		cs(text, "琴理", Text, "琴理");
		cs(text, "男", Text, "男子");
		cs(text, "雨宮", Text, "雨宫");
		cs(text, "神谷", Text, "神谷");
		cs(text, "ジン", Text, "Jin");
		cs(text, "建内", Text, "建内");
		cs(text, "少年", Text, "少年");
		cs(text, "客", Text, "客");
		cs(text, "異神", Text, "异神");
		cs(text, "警官", Text, "警官");
		cs(text, "野次馬", Text, "围观者");
		cs(text, "姫路", Text, "姬路");
		cs(text, "教師", Text, "老师");
		cs(text, "アナウンサー", Text, "アナウンサー");
		cs(text, "学園長", Text, "学园长");
		cs(text, "クラスメイト", Text, "同学");
	}
}

PVOID pGetText = (PVOID)(BaseAddr + 0x32C8);
__declspec(naked)void mGetText()
{
	__asm
	{
		pushad
		pushfd
		push esi
		call AymGetText
		popfd
		popad
		jmp pGetText
	}
}

int (*sub_414810)(int String, int a2, LPCSTR lpString, int a4, int a5);
int MyHook(int String, int a2, LPCSTR lpString, int a4, int a5)
{
	//cout << lpString << endl;
	if (!strcmp(lpString, "僎乕儉傪廔椆偟傑偡丅傛傠偟偄偱偡偐丠"))
		strcpy((char*)lpString, "　　　　即将结束游戏，确认吗？");
	if (!strcmp(lpString, "要回到标题界面吗？"))
		strcpy((char*)lpString, "即将返回标题，确认吗？");
	return sub_414810(String, a2, lpString, a4, a5);
}

void __stdcall AymGetTextFromMem(char* text)
{
	//cout << text << endl;
	/*
	if (!strcmp(text, "\x81\x45"))
		strcpy(text, "·");
	if (!strcmp(text, "\x81\x40"))
		strcpy(text, "　");
		*/
	for (int i = 0; i < strlen(text); i++)
	{
		if (text[i] == '\x81' && text[i + 1] == '\x45')
		{
			text[i] = '\xa1';
			text[i + 1] = '\xa4';
		}
		if (text[i] == '\x81' && text[i + 1] == '\x40')
		{
			text[i] = '\xa1';
			text[i + 1] = '\xa1';
		}

	}
	//cout << text << endl;
}

PVOID pGetTextFromMem = (PVOID)(BaseAddr + 0x18813);
__declspec(naked)void mGetTextFromMem()
{
	__asm
	{
		pushad
		pushfd
		push esi
		call AymGetTextFromMem
		popfd
		popad
		jmp pGetTextFromMem
	}
}

char* (*sub_448B00)(char* a1, int a2);
char* mh(char* a1, int a2)
{
	//cout << a1 << endl;
	//cout << a2 << endl;
	return sub_448B00(a1, a2);
}

PVOID pByPassDebugMod = (PVOID)(BaseAddr + 0x27E8B);
__declspec(naked)void mByPassDebugMod()
{
	__asm
	{
		pushad
		pushfd
		mov eax ,0x1
		mov dword ptr ds : [0x488AE0],eax
		popfd
		popad
		jmp pByPassDebugMod
	}
}

void Hook2()
{
	* (DWORD*)&sub_446DF0 = (BaseAddr + 0x46DF0);
	* (DWORD*)&sub_40B000 = (BaseAddr + 0xB000);
	* (DWORD*)&sub_414810 = (BaseAddr + 0x14810);
	* (DWORD*)&sub_448B00 = (BaseAddr + 0x48B00);
	DetourTransactionBegin();
	DetourAttach((void**)&sub_446DF0, MyFileHook);
	DetourAttach((void**)&sub_40B000, MyMjoSuppost);
	DetourAttach((void**)&sub_414810, MyHook);
	DetourAttach((void**)&pGetText, mGetText);
	DetourAttach((void**)&pGetTextFromMem, mGetTextFromMem);
	DetourAttach((void**)&pByPassDebugMod, mByPassDebugMod);
	DetourAttach((void**)&sub_448B00, mh);
	DetourTransactionCommit();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		//make_console();
		Hook();
		Hook2();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
__declspec(dllexport)void WINAPI Dummy()
{
}

