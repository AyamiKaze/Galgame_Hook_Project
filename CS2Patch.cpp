// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "detours.h"
#pragma comment(lib, "detours.lib")
using namespace std;

typedef bool (WINAPI* fnSetWindowTextW)(HWND hWnd, LPCWSTR lpString);
typedef int (WINAPI* fnMessageboxW)(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_     UINT    uType
	);
fnMessageboxW MessageBoxWOLD;
fnSetWindowTextW pSetWindowTextW;
typedef HFONT(WINAPI* fnCreateFontIndirectW)(LOGFONTW* lplf);
fnCreateFontIndirectW pCreateFontIndirectW;
PVOID g_pOldMultiByteToWideChar = NULL;
typedef int(WINAPI* PfuncMultiByteToWideChar)(
	_In_      UINT   CodePage,
	_In_      DWORD  dwFlags,
	_In_      LPCSTR lpMultiByteStr,
	_In_      int    cbMultiByte,
	_Out_opt_ LPWSTR lpWideCharStr,
	_In_      int    cchWideChar);
/*
void BypassInstallCheck()
{
	BYTE InstallCheckJMP[] = { 0xEB };
	int CheckedJMP = BaseAddr + 0x12F1A3;
	memcopy((void*)CheckedJMP, InstallCheckJMP, sizeof(InstallCheckJMP));
}

void MyCharSet()
{
	BYTE CharSet[] = { 0xFF };
	/*
	int MyCharSet1 = BaseAddr + 0x155AD3;
	int MyCharSet2 = BaseAddr + 0x155ADB;
	int MyCharSet3 = BaseAddr + 0x155AE6;
	int MyCharSet4 = BaseAddr + 0x155AEE;
	//
	int MyCharSet1 = BaseAddr + 0x273EA3;
	int MyCharSet2 = BaseAddr + 0x273EAB;
	int MyCharSet3 = BaseAddr + 0x273EB6;
	int MyCharSet4 = BaseAddr + 0x273EBE;
	memcopy((void*)MyCharSet1, CharSet, sizeof(CharSet));
	memcopy((void*)MyCharSet2, CharSet, sizeof(CharSet));
	//memcopy((void*)MyCharSet3, CharSet, sizeof(CharSet));
	memcopy((void*)MyCharSet4, CharSet, sizeof(CharSet));
}

void CatSystemInitPatch()
{
	BypassInstallCheck();
	MyCharSet();
}
*/

bool WINAPI newSetWindowTextW(HWND hw, LPCWSTR lps)
{
	if (!lstrcmpW(lps, L"密语")) 
	{
		wchar_t newtitle[] = L"【绿茶汉化组】密语 - v1.1";
		return pSetWindowTextW(hw, newtitle);
	}
	return pSetWindowTextW(hw, lps);
}

int WINAPI NewMultiByteToWideChar(UINT cp, DWORD dwFg, LPCSTR lpMBS, int cbMB, LPWSTR lpWCS, int ccWC)
{
	if(cp==932)
		cp=936;
	return ((PfuncMultiByteToWideChar)g_pOldMultiByteToWideChar)(cp, dwFg, lpMBS, cbMB, lpWCS, ccWC);
}

HFONT WINAPI newCreateFontIndirectW(LOGFONTW* lplf)
{
	lplf->lfCharSet = GB2312_CHARSET;
	//メイリオ
	if(!lstrcmpW(lplf->lfFaceName, L"メイリオ"))
		wcscpy(lplf->lfFaceName, L"SimHei");
	return pCreateFontIndirectW(lplf);
}

int WINAPI NEWMessageBoxW(_In_opt_ HWND    hWnd,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_     UINT    uType)
{
	if (!lstrcmpW(lpCaption, L"終了"))
		return MessageBoxWOLD(hWnd, L"即将结束游戏，是否确认", L"确认", uType);
	return MessageBoxWOLD(hWnd, lpText, lpCaption, uType);
}

int __stdcall sub_674D10(const LOGFONTW* a1, const TEXTMETRICW* a2, DWORD a3, LPARAM a4)
{
	const WCHAR* v4; // ecx
	v4 = a1->lfFaceName;
	if (*v4 != 64 && a1->lfCharSet == 0x86)
	{
		cout << wtoc(v4) << endl;
		lstrcpyW((LPWSTR)(*(DWORD*)(a4 + 16) + (*(DWORD*)(a4 + 12) << 7)), v4);
		++* (DWORD*)(a4 + 12);
		return 1;
	}
	//cout << "Hook" << endl;
	return 1;
}

typedef int (WINAPI* fnEnumFontFamiliesExW)(
	HDC           hdc,
	LPLOGFONTW    lpLogfont,
	FONTENUMPROCW lpProc,
	LPARAM        lParam,
	DWORD         dwFlags
	);
fnEnumFontFamiliesExW pEnumFontFamiliesExW;
int WINAPI newEnumFontFamiliesExW(HDC hdc, LPLOGFONTW lpLogfont, FONTENUMPROCW lpProc, LPARAM lParam, DWORD dwFlags)
{
	//lpLogfont->lfCharSet = GB2312_CHARSET;
	//cout << lpProc << endl;
	if((DWORD)lpProc == (BaseAddr+0x274D10))
		return ((fnEnumFontFamiliesExW)pEnumFontFamiliesExW)(hdc, lpLogfont, sub_674D10, lParam, dwFlags);
	return ((fnEnumFontFamiliesExW)pEnumFontFamiliesExW)(hdc, lpLogfont, lpProc, lParam, dwFlags);
}

void Init()
{
	pSetWindowTextW = (fnSetWindowTextW)GetProcAddress(GetModuleHandle(L"User32.dll"), "SetWindowTextW");
	MessageBoxWOLD = (fnMessageboxW)GetProcAddress(GetModuleHandle(L"User32.dll"), "MessageBoxW");
	pCreateFontIndirectW = (fnCreateFontIndirectW)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "CreateFontIndirectW");
	g_pOldMultiByteToWideChar = DetourFindFunction("Kernel32.dll", "MultiByteToWideChar");
	pEnumFontFamiliesExW = (fnEnumFontFamiliesExW)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "EnumFontFamiliesExW");
	DetourTransactionBegin();
	DetourAttach((void**)& pSetWindowTextW, newSetWindowTextW);
	DetourAttach((void**)&pCreateFontIndirectW, newCreateFontIndirectW);
	DetourAttach((void**)&MessageBoxWOLD, NEWMessageBoxW);
	DetourAttach((void**)&pEnumFontFamiliesExW, newEnumFontFamiliesExW);
	DetourAttach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);
	if (DetourTransactionCommit() != NO_ERROR)
	{
		MessageBoxW(NULL, L"API Function Hook Error.", L"Init", MB_OK | MB_ICONERROR);
	}
}

/*
0066E48F  |.  3942 04       |cmp dword ptr ds:[edx+0x4],eax
0066E492  |.  7E 37         |jle short cs2_crac.0066E4CB
0066E494  |>  8B5424 5C     |/mov edx,dword ptr ss:[esp+0x5C]
0066E498  |.  85D2          ||test edx,edx
0066E49A  |.  74 1D         ||je short cs2_crac.0066E4B9
0066E49C  |.  8D6424 00     ||lea esp,dword ptr ss:[esp]
0066E4A0  |>  0FB603        ||/movzx eax,byte ptr ds:[ebx]
0066E4A3  |.  8D49 01       |||lea ecx,dword ptr ds:[ecx+0x1]
0066E4A6  |.  43            |||inc ebx
0066E4A7  |.  8A4430 78     |||mov al,byte ptr ds:[eax+esi+0x78]
0066E4AB  |.  02440F FF     |||add al,byte ptr ds:[edi+ecx-0x1]
0066E4AF  |.  8841 FF       |||mov byte ptr ds:[ecx-0x1],al
0066E4B2  |.  4A            |||dec edx
0066E4B3  |.^ 75 EB         ||\jnz short cs2_crac.0066E4A0
0066E4B5  |.  8B4424 18     ||mov eax,dword ptr ss:[esp+0x18]
0066E4B9  |>  8B5424 24     ||mov edx,dword ptr ss:[esp+0x24]
0066E4BD  |.  40            ||inc eax
0066E4BE  |.  2BEF          ||sub ebp,edi
0066E4C0  |.  894424 18     ||mov dword ptr ss:[esp+0x18],eax
0066E4C4  |.  8BCD          ||mov ecx,ebp
0066E4C6  |.  3B42 04       ||cmp eax,dword ptr ds:[edx+0x4]
0066E4C9  |.^ 7C C9         |\jl short cs2_crac.0066E494
0066E4CB  |>  8B4424 54     |mov eax,dword ptr ss:[esp+0x54]         ;  hook here
*/


#define IMAGE_CRC_INIT (0L)

wchar_t* szHG3Name = NULL;
unsigned int szHG3IndexName = NULL;

bool ReadPngFile(const std::string& strName, int nOffset, PNG_DATA* pPngData)
{
	if (!pPngData || strName.empty())
	{
		return false;
	}

	FILE* fPng = fopen(strName.c_str(), "rb");
	if (!fPng)
	{
		return false;
	}

	if (nOffset)
	{
		fseek(fPng, nOffset, SEEK_SET);
	}

	if (!PngFile::ReadPngFile(fPng, pPngData))
	{
		fclose(fPng);
		if (pPngData->pRgba)
		{
			free(pPngData->pRgba);
		}

		MessageBoxA(NULL, strName.c_str(), "PNG Error", MB_OK);
		return false;
	}

	fclose(fPng);
	return true;
}

void ReplaceImage(BYTE* pBuffer, unsigned int nWidth, unsigned int nHeight, char* FileName)
{
	if (!pBuffer)
	{
		return;
	}

	unsigned int nImageSize = nWidth * nHeight * 4;
	PNG_DATA oPngData = { 0 };

	if (ReadPngFile(FileName, 0, &oPngData))
	{
		memcpy(pBuffer, oPngData.pRgba, nImageSize);
		free(oPngData.pRgba);
		cout << "copied" << endl;
		return;
	}
}

void WriteImage(BYTE* pBuffer, unsigned int nWidth, unsigned int nHeight)
{
	unsigned int nImageSize = nWidth * nHeight * 4;

	PNG_DATA pic = { 0 };
	pic.eFlag = HAVE_ALPHA;
	pic.nWidth = nWidth;
	pic.nHeight = nHeight;
	pic.pRgba = pBuffer;

	unsigned int nCrc = crc32(IMAGE_CRC_INIT, pBuffer, nImageSize);
	char szImageName[MAX_PATH] = { 0 };
	sprintf(szImageName, "image_cn\\%08X.png", nCrc);
	std::string strName(szImageName);
	PngFile::WritePngFile(strName.c_str(), &pic);
}

void __stdcall ProcessImage(BYTE* pBuffer, unsigned int nWidth, unsigned int nHeight)
{
	if (szHG3Name)
	{
		cout << "-----------------------------Get image info-----------------------------" << endl;
		char FileName[MAX_PATH] = { 0 };
		sprintf(FileName, "image_cn\\%s\\%s_%d.png", wtoc(szHG3Name), wtoc(szHG3Name), szHG3IndexName);
		cout << FileName << endl;
		cout << "Width:" << nWidth << endl;
		cout << "Height:" << nHeight << endl;
		szHG3Name = NULL;
		szHG3IndexName = NULL;
		//WriteImage(pBuffer, nWidth, nHeight);
		ReplaceImage(pBuffer, nWidth, nHeight, FileName);
		cout << "-----------------------------Get image info-----------------------------" << endl;
	}
	return;
}

void* g_GetHG3Name = (void*)(BaseAddr + 0x1f1d22);
void __declspec(naked) GetHG3Name()
{
	__asm
	{
		mov szHG3Name,eax
		jmp g_GetHG3Name
	}
}
void* g_GetHG3IndexName = (void*)(BaseAddr + 0x25a0aa);
void __declspec(naked) GetHG3IndexName()
{
	__asm
	{
		mov ecx, [esp + 0x24]
		mov szHG3IndexName, ecx
		jmp g_GetHG3IndexName
	}
}

void* g_p_copy_image = (void*)(BaseAddr + 0x26e4cb);
void __declspec(naked) copy_image()
{
	__asm
	{
		pushad
		pushfd

		mov eax, dword ptr[esp + 0x48]
		mov	ebx, dword ptr[eax + 0x4]  //ebx = height
		mov edx, dword ptr[esp + 0x80]
		add ecx, edx  //ecx = buffer
		shr edx, 2  //edx = width
		push ebx
		push edx
		push ecx
		call ProcessImage

		popfd
		popad
		jmp g_p_copy_image
	}
}

void ImagePatch()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&g_p_copy_image, copy_image);
	DetourAttach(&g_GetHG3IndexName, GetHG3IndexName);
	DetourAttach(&g_GetHG3Name, GetHG3Name);

	if (DetourTransactionCommit() != NO_ERROR)
	{
		MessageBoxW(NULL, L"Hook Error.", L"ImagePatch", MB_OK | MB_ICONERROR);
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
		Init();
		ImagePatch();
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