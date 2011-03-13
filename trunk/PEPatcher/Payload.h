#pragma once

#include <windows.h>

#pragma pack(push)
#pragma pack(1)
struct PushCode
{
	PushCode()
	{
		code = 0x68;
	}

	BYTE code;
};

struct CallCode
{
	CallCode()
	{
		code = 0xE8;
	}
	BYTE code;
};

struct JmpCode
{
	JmpCode()
	{
		code = 0xE9;
	}
	BYTE code;
};

//jmp dword ptr [xxxx]
struct JmpDwordPtrInd
{
	JmpDwordPtrInd()
	{
		code1 = 0xFF;
		code2 = 0x25;
	}

	BYTE code1;
	BYTE code2;
};

struct MessageBoxPayload
{
	MessageBoxPayload(DWORD imageBase,
					  DWORD payloadOffset,
					  DWORD oep,
					  DWORD messageBoxImportThunkAddress,
					  DWORD captionSize)
	{
		BYTE* firstPayloadCommand = reinterpret_cast<BYTE*>(&_pushMbOk);
		_mbOk = MB_OK;
		_caption = imageBase + payloadOffset + sizeof(MessageBoxPayload);
		_text = _caption + captionSize;
		_oep = oep - payloadOffset - ((BYTE*)&_messageBoxJmpCode - firstPayloadCommand);
		_messageBoxImportThunkAddress = imageBase + messageBoxImportThunkAddress;
		_hwnd = 0;
		_jmpToMessageBoxOffset = (BYTE*)&_messageBoxJmpCode - (BYTE*)&_jmp;
	}
	//push MB_OK
	PushCode _pushMbOk;
	DWORD _mbOk;
	// push "Hello"
	PushCode _pushCaption;
	DWORD _caption;
	// push "Hello from Kori!"
	PushCode _pushText;
	DWORD _text;
	// push 0
	PushCode _pushHWND;
	DWORD _hwnd;
	// call <jmp user32.MessageBoxA>
	CallCode _callMessageBox;
	DWORD _jmpToMessageBoxOffset;
	// jmp OEP
	JmpCode _jmp;
	DWORD   _oep;
	// jmp user32.MessageBoxA
	JmpDwordPtrInd _messageBoxJmpCode;
	DWORD _messageBoxImportThunkAddress;
	// ƒанные идут сразу после последней команды
};
#pragma pack(pop)
