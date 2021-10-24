// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"
#define shion
#define HEAD_STRING "$TEXT_LIST__"

vector <char*> new_text;
/// Char Path Set
#define SYS_ReadUInt32(pt,uv) \
	uv = *(unsigned int*)pt; \
	pt += sizeof(unsigned int) 


char* SYS_ReadString(unsigned char** pt)
{
	char* ptx;
	ptx = (char*)*pt;
	*pt += strlen(ptx) + 1;

	return ptx;
}

void SYS_DecodeTextFile(unsigned char* c)
{
	unsigned char* pt;
	unsigned int strcount;
	unsigned int strid;

	unsigned int curid;
	char* cursc;

	pt = c;

	pt += sizeof(HEAD_STRING) - 1;

	SYS_ReadUInt32(pt, strcount);


	for (strid = 0; strid < strcount; strid++)
	{
		SYS_ReadUInt32(pt, curid);
		cursc = SYS_ReadString(&pt);
		new_text.push_back(cursc);

	}
}

VOID SYS_LoadText()
{
#ifdef tsubaki
	FILE* fp = fopen("tsubaki.dat", "rb");
#else
	FILE* fp = fopen("shion.dat", "rb");
#endif
	if (!fp)
	{
		MessageBox(0, L"读取汉化文本失败", L"SYS_LoadText", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}
	dat_header_t hdr;
	fseek(fp, 0, SEEK_END);
	DWORD size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fread(&hdr, sizeof(hdr), 1, fp);
	fseek(fp, sizeof(hdr), SEEK_SET);
	char* buff = (char*)malloc(hdr.size);
	if (!buff)
	{
		MessageBox(0, L"内存分配失败", L"SYS_LoadText", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}
	fread(buff, hdr.size, 1, fp);
	fclose(fp);
	for (unsigned int i = 0; i < hdr.size; i++)
	{
		buff[i] -= 0x20;
		buff[i] ^= 0xFF;
	}
	unsigned long long BuffHash = MurmurHash64B(buff, hdr.size);
	if (BuffHash == hdr.hash)
	{
		SYS_DecodeTextFile((unsigned char*)buff);
	}
	else
	{
		MessageBox(0, L"文本校验失败", L"SYS_LoadText", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}

}

/// API Hook
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
char* (*GetTextByIndexOld)(int v1, int v2, int v3, int v4);
char* RepString(int v1, int v2, int v3, int v4)
{

	char* ret;
	int index;
	ret = GetTextByIndexOld(v1, v2, v3, v4);
	index = *(int*)ret;
	/*
	if ((v3 & 0x10000000) || v4 == 0xFFFFFFF) 
	{
		//wchar_t err[MAX_PATH];
		//wsprintf(err, L"读取到意外的参数:\nv3:%x\nv4:%x", v3, v4);
		//MessageBox(0, L"读取到意外的参数", L"GetTextByIndex", MB_OK | MB_ICONINFORMATION);
		return ret;
	}
	*/
	index = *(int*)ret;
	char* s_start = ret + sizeof(int);

	strcpy(s_start, new_text[index]);
	//cout << s_start << endl;
	return ret;
}


HFONT WINAPI CreateFontAEx(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace)
{
	if (fdwCharSet == 0x80)
	{
		if (strcmp(lpszFace, "俵俽 僑僔僢僋") == 0)
		{
			fdwCharSet = 0x86;
			return CreateFontAOLD(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic, 
				fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, "黑体");
		}
		else if (strcmp(lpszFace, "俵俽 柧挬") == 0)
		{
		defualt:
			fdwCharSet = 0x86;
			return CreateFontAOLD(nHeight, nWidth, nEscapement, nOrientation, fnWeight, fdwItalic,
				fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, "宋体");
		}
		else
			goto defualt;
	}
	goto defualt;
}

PVOID g_pOldCreateFontIndirectA = NULL;
typedef int (WINAPI* PfuncCreateFontIndirectA)(LOGFONTA* lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA* lplf)
{
	lplf->lfCharSet = GB2312_CHARSET;

	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

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
	//cout << Pstr << endl;
	if (strcmp(Pstr, "本当に終了しますか？") == 0)
	{
		strcpy(Pstr, "确认要结束游戏吗？");
	}
	return MessageBoxAOLD(hWnd, Pstr, "提示", uType);
}

PVOID g_pOldSetWindowTextA = NULL;
typedef int (WINAPI* PfuncSetWindowTextA)(HWND hwnd, LPCTSTR lpString);
int WINAPI NewSetWindowTextA(HWND hwnd, LPCTSTR lpString)
{
	if (strcmp((char*)(LPCTSTR)lpString, "弶婲摦帪偺僂僀儞僪僂儌乕僪傪愝掕偟傑偡丅") == 0)
	{
		return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, (LPCTSTR)(char*)"第一次启动游戏请选择显示模式。");
	}
	else if (strcmp((char*)(LPCTSTR)lpString, "僼儖僗僋儕乕儞偱僎乕儉傪奐巒偡傞丅") == 0)
	{
		return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, (LPCTSTR)(char*)"以全屏模式启动");
	}
	else
	{
		return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
	}
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
#ifdef tsubaki
	const char* szWndName = "【天之圣杯汉化组】与椿同住 共结情缘的剑舞恋曲SS - v1.0";
#else
	const char* szWndName = "【天之圣杯汉化组】与紫苑同住 共结情缘的剑舞恋曲SS - v1.0";
#endif

	return ((pfuncCreateWindowExA)g_pOldCreateWindowExA)(dwExStyle, lpClassName, (LPCTSTR)szWndName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

void Init()
{
#ifdef tsubaki
	*(DWORD*)&GetTextByIndexOld = BaseAddr + 0x58840;
#else
	*(DWORD*)&GetTextByIndexOld = BaseAddr + 0x58840;
#endif
	cout << "MainAddress:0x00" << hex << (BaseAddr) << endl;
	cout << "HookAddress:0x" << hex << (GetTextByIndexOld) << endl;
	CreateFontAOLD = (fnCreateFontA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontA");
	MessageBoxAOLD = (fnMessageboxA)GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxA");
	g_pOldSetWindowTextA = DetourFindFunction("USER32.dll", "SetWindowTextA");
	g_pOldCreateFontIndirectA = DetourFindFunction("GDI32.dll", "CreateFontIndirectA");
	DetourTransactionBegin();
	DetourAttach((void**)&GetTextByIndexOld, RepString);
	DetourAttach((void**)&CreateFontAOLD, CreateFontAEx);
	DetourAttach((void**)&MessageBoxAOLD, MessageBoxAEx);
	DetourAttach(&g_pOldSetWindowTextA, NewSetWindowTextA);
	DetourAttach(&g_pOldCreateWindowExA, NewCreateWindowExA);

	if (DetourTransactionCommit() != NOERROR)
	{
		MessageBox(0, L"API hook失败", L"Init", MB_OK | MB_ICONERROR);
		ExitProcess(-1);
	}
}


// PAL


int (*PalSpriteLoadMemory)(int a1, int FileName, int FileBuff, int FileSize, int a5);
int EdenSpriteLoadMemory(int a1, int FileName, int FileBuff, int FileSize, int a5)
{
	/*
	string fnm((char*)FileName);
	string Dir = "tsubaki_pic\\";
	string OutName = Dir + fnm;
	*/
	char fnm[MAX_PATH];
#ifdef tsubaki
	sprintf(fnm, "tsubaki_pic\\%s", (char*)FileName);
#else
	sprintf(fnm, "shion_pic\\%s", (char*)FileName);
#endif
	FILE* f = fopen(fnm, "rb");
	cout << "Read PGD:" << fnm << endl;
	if (f != NULL)
	{
		DWORD size = 0;
		char* buff = NULL;
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		buff = (char*)malloc(size);
		if (buff != NULL && size != 0) {
			fread(buff, size, 1, f);
			FileBuff = (int)buff;
			FileSize = (int)size;
			fclose(f);
			buff = NULL;
			size = 0;
			cout << "Read Out PGD File:" << (char*)FileName << " success." << endl;
		}
		else
		{
			fclose(f);
			buff = NULL;
			size = 0;
		}
		goto retn;
	}
retn:
	return PalSpriteLoadMemory(a1, FileName, FileBuff, FileSize, a5);
}
bool IsOnce = false;
typedef HMODULE(WINAPI* PfuncLoadLibraryExA)(
	LPCSTR lpLibFileName,
	HANDLE HfILE,
	DWORD dwFlags
	);
PfuncLoadLibraryExA LoadLibraryExA_Old;
HMODULE WINAPI MyLLEA(
	LPCSTR lpLibFileName,
	HANDLE HfILE,
	DWORD dwFlags
)
{
	HMODULE Head = LoadLibraryExA_Old(lpLibFileName, HfILE, dwFlags);
	DWORD Addr = (DWORD)Head;
	string LibName(lpLibFileName);
	string strName = LibName.substr(LibName.find_last_of("\\") + 1);
	if (strName == "PAL.dll" && !IsOnce)
	{
		/// path for pal.dll
		IsOnce = true;
		BYTE* pPalFontSetType = (BYTE*)GetProcAddress(Head,"PalFontSetType");
		if (pPalFontSetType) 
		{
			MEM_SetNopCode(&pPalFontSetType[0xE], 2);
			MEM_SetNopCode(&pPalFontSetType[0x17], 2);
		}

		* (DWORD*)&PalSpriteLoadMemory = (DWORD)GetProcAddress(Head, "PalSpriteLoadMemory");//Addr + 0xB5C60;
		cout << "PalSpriteLoadMemory:0x" << hex << PalSpriteLoadMemory << endl;
		DetourTransactionBegin();
		DetourAttach((void**)&PalSpriteLoadMemory, EdenSpriteLoadMemory);
		if (DetourTransactionCommit() != NOERROR)
		{
			MessageBox(0, L"PAL func hook失败", L"PalPatch", MB_OK | MB_ICONERROR);
			ExitProcess(-1);
		}
	}
	return Head;
}

void GetPalAddr()
{
	LoadLibraryExA_Old = (PfuncLoadLibraryExA)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryExA");
	DetourTransactionBegin();
	DetourAttach((void**)&LoadLibraryExA_Old, MyLLEA);
	if (DetourTransactionCommit() != NOERROR)
	{
		MessageBox(0, L"PAL hook失败", L"GetPalAddr", MB_OK | MB_ICONERROR);
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
		//make_console();
		SYS_LoadText();
		Init();
		GetPalAddr();
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