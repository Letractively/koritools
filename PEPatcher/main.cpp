// PEPatcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "PEPatcher.h"
#include "Payload.h"

#include <iostream>

bool CheckCanBePatched(const wchar_t* fileName)
{
	PEImagePatcher patcher;
	
	if( !patcher.Open(fileName, true) )
	{
		std::wcout << L"Can't open file " << fileName << std::endl;
		return false;
	}
	
	if( !patcher.IsValidPE() )
	{
		std::cout << "File is not valid PE." << std::cout;
		return false;
	}

	if( patcher.HasOverlay() )
	{
		std::cout << "File has overlay. It will be better if we don't patch it." << std::endl;
		return false;
	}
	if( !patcher.Is32Exe() || !patcher.HasUsualSubsystem() ||
		patcher.HasDataDir(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR)
		)
	{
		std::cout << "Sorry, this type of executables is not supported yet. Patience." << std::endl;
		return false;
	}
	if( patcher.LastSectionVirtualSizeGreaterThanPhysical() )
	{
		std::cout << "Can't patch file using resize last section method. Sorry." << std::endl;
		return false;
	}

    DWORD messageBoxThunk = patcher.GetImportTableThunk("user32.dll", "MessageBoxA");
	if( messageBoxThunk )
	{
		return true;
	}
	DWORD messageBoxWThunk = patcher.GetImportTableThunk("user32.dll", "MessageBoxW");
	if( messageBoxWThunk )
	{
		return true;
	}
	if( patcher.CheckIfImportTableThunkCanBeAdded("user32.dll") )
	{
		return true;
	}
	std::cout << "Image doesn't import necessary for payload functions and we can't add them into import table." << std::endl;
	return false;
}

void Patch(const wchar_t* fileName, const wchar_t* mbCaption, const wchar_t* mbText)
{
	PEImagePatcher patcher;

	if(!patcher.Open(fileName, false))
	{
		std::wcout << "Can't open file " << fileName << " for writing." << std::endl;
		return ;
	}
	bool ansiVersion = true;
	// ���� ����� MessageBoxA
	DWORD messageBoxThunk = patcher.GetImportTableThunk("user32.dll", "MessageBoxA");
	if(!messageBoxThunk)
	{
		// ���� �� �������, ���� W ������ ������� ...
		messageBoxThunk = patcher.GetImportTableThunk("user32.dll", "MessageBoxW");
		if( messageBoxThunk)
		{
			ansiVersion = false;
		}
		else
		{
			// ���� � �� ���, �� ������� �������� � ������
			patcher.AddImportTableThunks("user32.dll", "MessageBoxA");
			messageBoxThunk = patcher.GetImportTableThunk("user32.dll", "MessageBoxA");
		}
	}
	// �������� �� ����������� �� ���� �� ������ ��� �������, � �������� �� � ������ �� �� �����
	if(!messageBoxThunk)
	{
		return ; // ������������ �� ��������
	}
	BYTE* pCaption = NULL;
	DWORD captionSize = 0;
	if( ansiVersion)
	{
		INT bytesReqired = WideCharToMultiByte(CP_ACP, 0, mbCaption, -1, NULL, 0, NULL, NULL);
		if(bytesReqired)
		{
			pCaption = new BYTE[bytesReqired]();
			WideCharToMultiByte(CP_ACP, 0, mbCaption, -1, (LPSTR)pCaption, bytesReqired, NULL, NULL);
			captionSize = bytesReqired;
		}
	}
	else
	{
		pCaption = (BYTE*)mbCaption;
		captionSize = (wcslen(mbCaption) + 1) * sizeof(wchar_t);
	}
	BYTE* pText = NULL;
	DWORD textSize = 0;
	if( ansiVersion)
	{
		INT bytesReqired = WideCharToMultiByte(CP_ACP, 0, mbText, -1, NULL, 0, NULL, NULL);
		if( bytesReqired)
		{
			pText = new BYTE[bytesReqired]();
			WideCharToMultiByte(CP_ACP, 0, mbText, -1, (LPSTR)pText, bytesReqired, NULL, NULL);
			textSize = bytesReqired;
		}
		
	}
	else
	{
		pText = (BYTE*)mbText;
		textSize = (wcslen(mbText) + 1) * sizeof(wchar_t);
	}
	// ���������, ��� �� ������������� ���������
	if( !pText || !pCaption || !captionSize || !textSize)
	{
		return ;
	}
	// ����� ��� ��� � ������
	DWORD payloadSize = sizeof(MessageBoxPayload) + captionSize + textSize;
	BYTE* payloadBuff = new BYTE[payloadSize](); 
	// ���
	MessageBoxPayload payload(patcher.GetBaseOfImage(), patcher.GetPayloadRVA(), patcher.GetEntryPoint(), messageBoxThunk, captionSize);
	// ���������� � ��� ����� ���
	memcpy(payloadBuff, &payload, sizeof(MessageBoxPayload));
	// ������ 1. ���������
	DWORD captionOffset = sizeof(MessageBoxPayload);
	memcpy(&payloadBuff[captionOffset], pCaption, captionOffset);
	// ������ 2. ����� ���������
	DWORD textOffset = sizeof(MessageBoxPayload) + captionSize;
	memcpy(&payloadBuff[textOffset], pText, textSize);
	
	// ������
	patcher.WritePayload(payloadBuff, payloadSize);

	//
	if(ansiVersion)
	{
		delete[] pCaption;
		delete[] pText;
	}
}

struct Params
{
	const wchar_t* inFileName;
	const wchar_t* outFileName;
	const wchar_t* text;
	const wchar_t* caption;
};

void PrintHelp()
{
	std::cout << "Usage: patcher.exe <in-file> [out out-file-name] [caption mb-caption text mb-text]" << std::endl;
}

const wchar_t* defaultCaption = L"Hello!";
const wchar_t* defaultText = L"Hello from Kori!!!";

bool ParseCommandLine(int argc, wchar_t* argv[], Params& params)
{
	memset(&params, 0, sizeof(Params));
	if(argc < 2 )
	{
		return false;
	}
	params.inFileName = argv[1];
	params.caption = defaultCaption;
	params.text = defaultText;
	// patcher.exe <in-file>
	if( argc == 2)
	{
		
		return true;
	}
	// patcher.exe <in-file> out <out-file>
	if( argc == 4 )
	{
		if( _wcsicmp(argv[2], L"out") != 0 )
		{
			return false;
		}
		params.outFileName = argv[3];
		return true;
	}
	// patcher.exe <in-file> caption <caption> text <text>
	if( argc == 6)
	{
		if( _wcsicmp(argv[2], L"caption") != 0 || _wcsicmp(argv[4], L"text") != 0)
		{
			return false;
		}
		params.caption = argv[3];
		params.text = argv[5];
		return true;
	}
	// patcher.exe <in-file> out <out-file> caption <cpation>  text <text>
	if( argc == 8)
	{
		if( _wcsicmp(argv[2], L"out") != 0 || _wcsicmp(argv[4], L"caption") != 0 || _wcsicmp(argv[6], L"text") != 0)
		{
			return false;
		}
		params.outFileName = argv[3];
		params.caption = argv[5];
		params.text = argv[7];
		return true;
	}
	return true;
}

int wmain(int argc, wchar_t* argv[])
{
	
	Params params;
	if( !ParseCommandLine(argc, argv, params) )
	{
		PrintHelp();
		return 0;
	}

	if( !CheckCanBePatched(params.inFileName) )
	{
		return 0;
	}
	
	if( params.outFileName )
	{
		if(!CopyFile(params.inFileName, params.outFileName, FALSE))
		{
			std::cout << "Can't create output file." << std::endl;
			return 1;
		}
	}
	
	Patch(params.outFileName ? params.outFileName : params.inFileName, params.caption, params.text);

	return 0;
}

