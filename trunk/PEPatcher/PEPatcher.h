#pragma once

#include <windows.h>

// ������ exe-����
// ��� ������ ��������������� 
// 1. ���������� ������������� ���� (payload)
// 2. ���� ��� ����������, ���������� ������ payload-y ������� ������� (�� ������ ��������)
// ���� �������������� ������� ���������� ��������� ������
class PEImagePatcher
{
public:
	PEImagePatcher();
	~PEImagePatcher();

	bool Open(const wchar_t* fileName, bool readOnly);
	void Close();
	
	// ���������, �������� �� ��� PE ����, � ����� �� � ��� ������ �����������
	bool IsValidPE();
	bool Is32Exe();
	bool HasUsualSubsystem();
	bool HasOverlay();
	bool HasDataDir(DWORD dataDir);
	bool LastSectionVirtualSizeGreaterThanPhysical();

	// ��������� ���� ������ ��� payload-a
	DWORD GetBaseOfImage();
	DWORD GetEntryPoint();
	// �����, �� �������� ����� ������� payload
	DWORD GetPayloadRVA(); 
	// ���������� ����� �� �������� ��������� ���������� ����� ������������� �������
	// 0 ���� ����� ������� �� �������������
	DWORD GetImportTableThunk(const char* dllName, const char* functionName);
	// ���������� Payload � ������ Entry Point
	DWORD WritePayload(const void* payload, DWORD payloadSize);
	
	// ������� �������� ������� � ������
	// �������� ������ ���� ������ ����� ������ �� ������������ �� ���� dll
	void AddImportTableThunks(const char* dllName, const char* functionName); 
	bool CheckIfImportTableThunkCanBeAdded(const char* dllName);

private:	
	HANDLE	_fileHandle;
	HANDLE	_fileMappingHandle;
	BYTE*	_fileContent;
	DWORD _fileSize;

	PIMAGE_NT_HEADERS _peHeader;
	PIMAGE_SECTION_HEADER _sectionList;
	PIMAGE_DATA_DIRECTORY _dataDirs;

	// ������ � ������
	bool OpenFile(const wchar_t* fileName, bool readOnly);
	void CloseFile();
	bool MapFile(bool readOnly);
	void UnmapFile();
	bool FetchPEHeaders();
	bool Resize(DWORD newFileSize);

	// �������� Entry Point
	void SetEnrtyPoint(DWORD ep); 

	// ��������������� ������ ��� ������ � ��������
	DWORD AlignUp(DWORD size, DWORD alignment);
	DWORD RvaToOffset(DWORD rva);
	
	// ������ � ��������
	DWORD ResizeLastSection(DWORD additionalSize);
	void SetLastSectionFlags(DWORD flags);
	DWORD GetSectionCount();

	// ��������������� ������ ��� ������ � ��������
	PIMAGE_IMPORT_DESCRIPTOR GetFirstImportDescriptor();
	PIMAGE_IMPORT_DESCRIPTOR GetImportDescriptorByName(const char* dllName);
	DWORD GetImportDescriprotCount();
};