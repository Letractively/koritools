#pragma once

#include <windows.h>

// Патчит exe-файл
// Под патчем подразумевается 
// 1. Добавление произвольного кода (payload)
// 2. Если это необходимо, добавление нужных payload-y функций импорта (не всегда возможно)
// Патч осуществляется методом расширения последней секции
class PEImagePatcher
{
public:
	PEImagePatcher();
	~PEImagePatcher();

	bool Open(const wchar_t* fileName, bool readOnly);
	void Close();
	
	// Проверяем, валидный ли это PE файл, и стоит ли с ним вообще связываться
	bool IsValidPE();
	bool Is32Exe();
	bool HasUsualSubsystem();
	bool HasOverlay();
	bool HasDataDir(DWORD dataDir);
	bool LastSectionVirtualSizeGreaterThanPhysical();

	// Получение инфы нужной для payload-a
	DWORD GetBaseOfImage();
	DWORD GetEntryPoint();
	// Адрес, по которому будет записан payload
	DWORD GetPayloadRVA(); 
	// Возвращает адрес по которому загрузчик записывает адрес импортируемой функции
	// 0 если такая функция не импортируется
	DWORD GetImportTableThunk(const char* dllName, const char* functionName);
	// Записывает Payload и правит Entry Point
	DWORD WritePayload(const void* payload, DWORD payloadSize);
	
	// Пробует добавить функцию в импорт
	// Работает только если модуль ранее ничего не импортировал из этой dll
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

	// Работа с файлом
	bool OpenFile(const wchar_t* fileName, bool readOnly);
	void CloseFile();
	bool MapFile(bool readOnly);
	void UnmapFile();
	bool FetchPEHeaders();
	bool Resize(DWORD newFileSize);

	// Изменяет Entry Point
	void SetEnrtyPoint(DWORD ep); 

	// Вспомогательные методы для работы с адресами
	DWORD AlignUp(DWORD size, DWORD alignment);
	DWORD RvaToOffset(DWORD rva);
	
	// Работа с секциями
	DWORD ResizeLastSection(DWORD additionalSize);
	void SetLastSectionFlags(DWORD flags);
	DWORD GetSectionCount();

	// Вспомогательные методы для работы с импортом
	PIMAGE_IMPORT_DESCRIPTOR GetFirstImportDescriptor();
	PIMAGE_IMPORT_DESCRIPTOR GetImportDescriptorByName(const char* dllName);
	DWORD GetImportDescriprotCount();
};