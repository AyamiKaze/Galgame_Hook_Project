// dllmain.cpp : 定义 DLL 应用程序的入口点。
/*
Ever tried. Ever failed.
No matter. Try again.
Fail again. Fail better.
*/
#include "pch.h"
HINSTANCE hInst;

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
PVOID g_pOldMultiByteToWideChar = NULL;
typedef int(WINAPI* PfuncMultiByteToWideChar)(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar);
wchar_t* ctow932(const char* str)
{
	wchar_t* buffer = nullptr;
	if (str)
	{
		size_t nu = strlen(str);
		size_t n = (size_t)MultiByteToWideChar(932, 0, (const char*)str, int(nu), NULL, 0);
		buffer = 0;
		buffer = new wchar_t[n];
		memset(buffer, 0, sizeof(wchar_t) * (n));
		MultiByteToWideChar(932, 0, (const char*)str, int(nu), buffer, int(n));
	}
	return buffer;
}

wchar_t* ctow936(const char* str)
{
	wchar_t* buffer = nullptr;
	if (str)
	{
		size_t nu = strlen(str);
		size_t n = (size_t)MultiByteToWideChar(936, 0, (const char*)str, int(nu), NULL, 0);
		buffer = 0;
		buffer = new wchar_t[n];
		memset(buffer, 0, sizeof(wchar_t) * (n));
		MultiByteToWideChar(936, 0, (const char*)str, int(nu), buffer, int(n));
	}
	return buffer;
}

LPWSTR ctowJIS(char* str)
{
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(932, 0, str, -1, NULL, 0); //计算长度
	LPWSTR out = new wchar_t[dwMinSize];
	MultiByteToWideChar(932, 0, str, -1, out, dwMinSize);//转换
	return out;
}

char* wtocGBK(LPCTSTR str)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(936, NULL, str, -1, NULL, 0, NULL, FALSE); //计算长度
	char* out = new char[dwMinSize];
	WideCharToMultiByte(936, NULL, str, -1, out, dwMinSize, NULL, FALSE);//转换
	return out;
}

ULONG a2u(
	LPCSTR lpAnsi,
	ULONG  Length,
	LPWSTR lpUnicodeBuffer,
	ULONG  BufferCount,
	ULONG  CodePage)
{
	return MultiByteToWideChar(CodePage, 0, lpAnsi, Length, lpUnicodeBuffer, BufferCount);
}

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
	wchar_t* wcs = nullptr;
	fdwCharSet = GB2312_CHARSET;
	char buffer[0x100] = { 0 };
	memset(buffer, 0, 0x100);
	memcpy(buffer, lpszFace, strlen(lpszFace));
	wcs = ctow936(buffer);
	return CreateFontW(nHeight, nWidth, nEscapement, nOrientation, fnWeight, 
	fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet, fdwOutputPrecision, 
	fdwClipPrecision, fdwQuality, fdwPitchAndFamily, wcs);
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
	wchar_t newtitle[] = L"【天之圣杯汉化组】奇异恩典·圣夜的小镇 - v1.0";
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

int WINAPI NewMultiByteToWideChar(UINT cp, DWORD dwFg, LPCSTR lpMBS, int cbMB, LPWSTR lpWCS, int ccWC)
{
	return ((PfuncMultiByteToWideChar)g_pOldMultiByteToWideChar)(936, dwFg, lpMBS, cbMB, lpWCS, ccWC);
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


void EdenInitHook()
{
	pCreateFontA = (fnCreateFontA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontA");
	pEnumFontFamiliesExA = (fnEnumFontFamiliesExA)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "EnumFontFamiliesExA");
	pGetSystemDefaultLangID = (fnGetSystemDefaultLangID)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetSystemDefaultLangID");
	pGetSystemDefaultUILanguage = (fnGetSystemDefaultUILanguage)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetSystemDefaultUILanguage");
	pGetUserDefaultLCID = (fnGetUserDefaultLCID)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetUserDefaultLCID");
	pSetWindowTextA = (fnSetWindowTextA)GetProcAddress(GetModuleHandle(L"User32.dll"), "SetWindowTextA");
	g_pOldMultiByteToWideChar = DetourFindFunction("Kernel32.dll", "MultiByteToWideChar");
	MessageBoxAOLD = (fnMessageboxA)GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxA");
	DetourTransactionBegin();
	DetourAttach((void**)& pCreateFontA, newCreateFontA);
	DetourAttach((void**)& pGetSystemDefaultLangID, newGetSystemDefaultLangID);
	DetourAttach((void**)& pGetSystemDefaultUILanguage, newGetSystemDefaultUILanguage);
	DetourAttach((void**)& pGetUserDefaultLCID, newGetUserDefaultLCID);
	DetourAttach((void**)& pSetWindowTextA, newSetWindowTextA);
	DetourAttach((void**)& pEnumFontFamiliesExA, newEnumFontFamiliesExA);
	DetourAttach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);
	DetourAttach((void**)& MessageBoxAOLD, MessageBoxAEx);
	if (DetourTransactionCommit() != NO_ERROR)
	{
		MessageBoxW(NULL, L"HOOK ERROR!!!", L"AymKze", MB_OK | MB_ICONWARNING);
	}
}
/**********************************************************************************************************************/
//中文显示
void EdenCharCheck()
{
	BYTE Patch1[] = { 0xFE };
	BYTE Patch2[] = { 0xFF };
	BYTE Patch3[] = { 0xA1 };

	int CheckPoint1 = BaseAddr + 0x349f5;
	int CheckPoint2 = BaseAddr + 0x349f9;
	int CheckPoint3 = BaseAddr + 0x349fd;
	int CheckPoint4 = BaseAddr + 0x359b8;
	int CheckPoint5 = BaseAddr + 0x359c2;
	int CheckPoint6 = BaseAddr + 0xaec61;
	int CheckPoint7 = BaseAddr + 0xaec6e;
	int SpacePoint1 = BaseAddr + 0x343db;
	int SpacePoint2 = BaseAddr + 0x343dc;

	if (debug) 
	{
		cout << "BGI Check info" << endl;
		cout << hex << CheckPoint1 << " | " << hex << CheckPoint2 << " | " << hex << CheckPoint3 << " | " << hex << CheckPoint4 << " | " << hex << CheckPoint5 << " | " << hex << CheckPoint6 << " | " << hex << CheckPoint7 << " | " << hex << SpacePoint1 << " | " << hex << SpacePoint2 << endl;
	}

	memcopy((void*)CheckPoint1, Patch2, sizeof(Patch2));
	memcopy((void*)CheckPoint2, Patch1, sizeof(Patch1));
	memcopy((void*)CheckPoint3, Patch2, sizeof(Patch2));
	memcopy((void*)CheckPoint4, Patch2, sizeof(Patch2));
	memcopy((void*)CheckPoint5, Patch2, sizeof(Patch2));
	memcopy((void*)CheckPoint6, Patch2, sizeof(Patch2));
	memcopy((void*)CheckPoint7, Patch2, sizeof(Patch2));
	memcopy((void*)SpacePoint1, Patch3, sizeof(Patch3));
	memcopy((void*)SpacePoint2, Patch3, sizeof(Patch3));
}

//////////////////////////////////FileSystem///////////////////////////////////
#ifndef ASM
#define ASM __declspec(naked)
#endif /* ASM */

#ifndef INLINE_ASM
#define INLINE_ASM __asm
#endif

#define LOOP_ONCE   for (Bool __condition_ = True; __condition_; __condition_ = False)

#define DIR L"amegure_cn\\"

DWORD ImmAlloc = (BaseAddr + 0xB4598);
DWORD ImmFree  = (BaseAddr + 0xB45A3);

ASM PVOID CDECL HostAlloc(LONG Size)
{
	INLINE_ASM
	{
		push dword ptr[esp + 4]
		call ImmAlloc
		pop ecx
		retn
	}
}


ASM VOID CDECL HostFree(PVOID lpMem)
{
	INLINE_ASM
	{
		push dword ptr[esp + 4]
		call ImmFree
		pop ecx
		retn
	}
}

wstring WINAPI GetPackageName(wstring& fileName)
{
	wstring temp(fileName);
	wstring::size_type pos = temp.find_last_of(L"\\");

	if (pos != wstring::npos)
	{
		temp = temp.substr(pos + 1, temp.length());
	}

	wstring temp2(temp);
	wstring::size_type pos2 = temp2.find_last_of(L"/");
	if (pos2 != wstring::npos)
	{
		temp2 = temp2.substr(pos + 1, temp2.length());
	}
	return temp2;
}
BOOL E(const wchar_t* what)
{
	MessageBox(0, what, L"AyamiKazeFileSystemError", 0);
	return FALSE;
}

#include "WinFile.h"
BOOL WINAPI AyamiKazeReadFileFromDir(LPCSTR lpFileName, PBYTE& Buffer, ULONG& OutSize)
{
	WCHAR   szFileName[MAX_PATH] = { 0 };
	a2u(lpFileName, lstrlenA(lpFileName), szFileName, MAX_PATH, 932);
	wstring fnm(szFileName);
	wstring FileName = GetPackageName(fnm);

	wstring Path = DIR + FileName;
	cout << "Reading file from:" << wtocGBK(Path.c_str()) << endl;
	WinFile File;
	if (File.Open(Path.c_str(), WinFile::FileRead) == S_FALSE)
	{
		cout << "Can't find file:" << wtocGBK(Path.c_str()) << ".Read file from BGI filesystem." << endl;
		File.Release();
		return FALSE;
	}
	OutSize = File.GetSize32();
	Buffer = (PBYTE)HostAlloc(OutSize);
	if (Buffer == nullptr)
	{
		File.Release();
		wchar_t what[MAX_PATH];
		wsprintf(what, FileName.c_str(), L":Alloc memmory error!");
		return E(what);
	}
	File.Read(Buffer, OutSize);
	File.Release();
	cout << "Success to read file from:" << wtocGBK(Path.c_str()) << endl;
	return TRUE;
}

LONG CDECL (*sub_46D740)(
	PVOID  pvDecompressed,
	PULONG pOutSize,
	ULONG  InSize,
	ULONG  SkipBytes,
	ULONG  OutBytes);
LONG CDECL AyamiKazeFileDIE(
	PVOID  pvDecompressed,
	PULONG pOutSize,
	ULONG  InSize,
	ULONG  SkipBytes,
	ULONG  OutBytes)
{
	return sub_46D740(pvDecompressed, pOutSize, InSize, SkipBytes, OutBytes);
}

//ReadFile:sub_46D9A0
LONG CDECL (*sub_46D9A0)(
	PVOID  pvDecompressed,
	PULONG pOutSize,
	ULONG  SkipBytes,
	ULONG  OutBytes
);
LONG CDECL AyamiKazeFileSystem(
	PVOID  pvDecompressed,
	PULONG pOutSize,
	ULONG  SkipBytes,
	ULONG  OutBytes
)
{
	char* tmpfile;
	LONG result = 5;
	ULONG InSize = 0;
	PBYTE InBuffer = nullptr;
	INLINE_ASM
	{
		mov tmpfile,ecx;
	}
	if (AyamiKazeReadFileFromDir(tmpfile,InBuffer,InSize))
	{
		INLINE_ASM mov ecx, InBuffer;
		result = AyamiKazeFileDIE(
			pvDecompressed,
			pOutSize,
			InSize,
			0,
			0);
		if (InBuffer)
			HostFree(InBuffer);
		return result;
	}
	else
	{
		INLINE_ASM
		{
			mov ecx,tmpfile;
		}
		return sub_46D9A0(pvDecompressed, pOutSize, SkipBytes, OutBytes);
	}
}

void FileSystemInit()
{
	*(DWORD*)&sub_46D740 = BaseAddr + 0x6D740;
	*(DWORD*)&sub_46D9A0 = BaseAddr + 0x6D9A0;
	cout << "Hooking function:0x" << hex << sub_46D740 << endl;
	cout << "Hooking function:0x" << hex << sub_46D9A0 << endl;
	DetourTransactionBegin();
	DetourAttach((void**)&sub_46D740, AyamiKazeFileDIE);
	DetourAttach((void**)&sub_46D9A0, AyamiKazeFileSystem);
	if (DetourTransactionCommit() != NOERROR)
		MessageBox(0, L"Init filesystem error.", L"InitFileSystem", 0);
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		if (debug)
		{
			make_console();
		}
		//EdenCharCheck();
		EdenInitHook();
		FileSystemInit();
		break;
    case DLL_THREAD_ATTACH:break;
    case DLL_THREAD_DETACH:break;
    case DLL_PROCESS_DETACH:break;
		//FileClose();
        break;
    }
    return TRUE;
}
__declspec(dllexport)void WINAPI CreateByAyamiKaze()
{
}
