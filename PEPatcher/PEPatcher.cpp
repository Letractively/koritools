#include "PEPatcher.h"

#include <cassert>

PEImagePatcher::PEImagePatcher()
:_fileHandle(INVALID_HANDLE_VALUE),
_fileMappingHandle(INVALID_HANDLE_VALUE),
_fileContent(NULL),
_fileSize(0)
{
}

PEImagePatcher::~PEImagePatcher()
{
	Close();
}

void PEImagePatcher::Close()
{
	UnmapFile();
	CloseFile();
}

bool PEImagePatcher::Open(const wchar_t* fileName, bool readOnly)
{
	Close();
	return OpenFile(fileName, readOnly) && MapFile(readOnly) && FetchPEHeaders();
}

bool PEImagePatcher::OpenFile(const wchar_t* fileName, bool readOnly)
{
	DWORD access = readOnly ? GENERIC_READ : FILE_ALL_ACCESS;
	DWORD shareMode = readOnly ? FILE_SHARE_READ : FILE_SHARE_WRITE;

	_fileHandle = CreateFileW(fileName, access, shareMode, NULL, OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE != _fileHandle)
	{
		_fileSize = GetFileSize(_fileHandle, 0);
		return true;
	}
	return false;
}

void PEImagePatcher::CloseFile()
{
	if(INVALID_HANDLE_VALUE != _fileHandle)
	{
		CloseHandle(_fileHandle);
		_fileHandle = INVALID_HANDLE_VALUE;
		_fileSize = 0;
	}
}

bool PEImagePatcher::MapFile(bool readOnly)
{
	if(INVALID_HANDLE_VALUE == _fileHandle)
	{
		return false;
	}
	DWORD protect = readOnly ? PAGE_READONLY : PAGE_READWRITE;
	DWORD mapAccess = readOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS;
	_fileMappingHandle = CreateFileMappingW(_fileHandle, NULL, protect, 0, 0, NULL);
	if(INVALID_HANDLE_VALUE != _fileMappingHandle)
	{
		_fileContent = static_cast<BYTE*>(MapViewOfFile(_fileMappingHandle, mapAccess, 0, 0, 0));
		DWORD lastError = GetLastError();
		return _fileContent != NULL;
	}
	return false;
}

void PEImagePatcher::UnmapFile()
{
	if(NULL != _fileContent)
	{
		UnmapViewOfFile(_fileContent);
		_fileContent = NULL;
	}
	if(INVALID_HANDLE_VALUE != _fileMappingHandle)
	{
		CloseHandle(_fileMappingHandle);
		_fileMappingHandle = INVALID_HANDLE_VALUE;
	}
}

bool PEImagePatcher::Resize(DWORD newFileSize)
{
	if(newFileSize == _fileSize)
	{
		return true;
	}
	UnmapFile();
	DWORD oldFileSize = _fileSize;
	assert( (newFileSize - _fileSize) >= 0 );
	if( INVALID_SET_FILE_POINTER == ::SetFilePointer(_fileHandle, newFileSize, 0, FILE_BEGIN) ||
		!::SetEndOfFile(_fileHandle) )
	{
		return false;
	}
	bool mapOk = MapFile(false);
	if(!mapOk)
	{
		return false;
	}
	if(!FetchPEHeaders())
	{
		return false;
	}
	memset(&_fileContent[oldFileSize], 0, newFileSize - oldFileSize);
	_fileSize = newFileSize;
	return true;
}

bool PEImagePatcher::FetchPEHeaders()
{
	assert( _fileContent );

	if( _fileSize < sizeof(IMAGE_DOS_HEADER) )
	{
		return false;
	}
	PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(_fileContent);
	// 1. Проверяем налицие инициалов Марка Зибковски
	if( IMAGE_DOS_SIGNATURE != pDosHeader->e_magic )
	{
		return false;
	}
	DWORD peHeaderOffset = pDosHeader->e_lfanew;
	// 
	if( peHeaderOffset > _fileSize ||
		(_fileSize - peHeaderOffset) < sizeof(IMAGE_NT_HEADERS) )
	{
		return false; 
	}
	_peHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(&_fileContent[peHeaderOffset]);
	// Проверяем сигнатуру PE
	if( IMAGE_NT_SIGNATURE != _peHeader->Signature )
	{
		_peHeader = NULL;
		return false;
	}
	// Проверяем сигнатуру 32-разрядного PE 64-разрядные экзешники пока не умеем обрабатывать
	DWORD sectionListOffset = peHeaderOffset + sizeof(IMAGE_NT_HEADERS);
	if( sectionListOffset > _fileSize ||
		(_fileSize - sectionListOffset) < sizeof(IMAGE_SECTION_HEADER) )
	{
		_peHeader = NULL;
		return false;
	}
	// Запоминаем указатель на список секций
	_sectionList = reinterpret_cast<PIMAGE_SECTION_HEADER>(&_fileContent[sectionListOffset]);
	// Запоминаем указатель на директории данных
	_dataDirs = _peHeader->OptionalHeader.DataDirectory;
	return true;
}

bool PEImagePatcher::IsValidPE()
{
	// Проверки на валидность заголовков в FetchPEHeaders
	return _peHeader != NULL;
}

bool PEImagePatcher::Is32Exe()
{
	assert(_peHeader);

	bool isExe = (_peHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) && 
		!(_peHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) &&
		!(_peHeader->FileHeader.Characteristics & IMAGE_FILE_SYSTEM);
	bool is32Exe = IMAGE_NT_OPTIONAL_HDR32_MAGIC == _peHeader->OptionalHeader.Magic;
	return isExe && is32Exe;
}

bool PEImagePatcher::HasUsualSubsystem()
{
	assert(_peHeader);

	WORD subsystem = _peHeader->OptionalHeader.Subsystem;
	// Оконное либо консольное приложение под Win
	return (IMAGE_SUBSYSTEM_WINDOWS_GUI == subsystem) || (IMAGE_SUBSYSTEM_WINDOWS_CUI == subsystem);
}

bool PEImagePatcher::HasOverlay()
{
	assert(_peHeader);

	DWORD size = _peHeader->OptionalHeader.SizeOfHeaders;
	for(DWORD i = 0; i != _peHeader->FileHeader.NumberOfSections; i++)
	{
		size += _sectionList[i].SizeOfRawData;
	}
	return size != _fileSize;
}

bool PEImagePatcher::HasDataDir(DWORD dataDir)
{
	assert(_peHeader);

	if( dataDir > _peHeader->OptionalHeader.NumberOfRvaAndSizes)
	{
		return false;
	}
	return _dataDirs[dataDir].VirtualAddress != NULL;
}

DWORD PEImagePatcher::GetSectionCount()
{
	assert(_peHeader);
	return _peHeader->FileHeader.NumberOfSections;
}

bool PEImagePatcher::LastSectionVirtualSizeGreaterThanPhysical()
{
	DWORD sectionNum = GetSectionCount();
	return _sectionList[sectionNum].Misc.VirtualSize > _sectionList[sectionNum].SizeOfRawData;
}

DWORD PEImagePatcher::RvaToOffset(DWORD rva)
{
	assert(_peHeader && _sectionList);

	DWORD result = 0;
	for(DWORD i = 0; i != _peHeader->FileHeader.NumberOfSections; i++)
	{
		if( rva >= _sectionList[i].VirtualAddress &&
			rva < (_sectionList[i].VirtualAddress + _sectionList[i].Misc.VirtualSize) 
		  )
		{
			if((rva - _sectionList[i].VirtualAddress) < _sectionList[i].SizeOfRawData)
			{
				result = _sectionList[i].PointerToRawData + (rva - _sectionList[i].VirtualAddress);
				break;
			}
		}
	}
	return result;
}

DWORD PEImagePatcher::ResizeLastSection(DWORD additionalSize)
{
	PIMAGE_SECTION_HEADER lastSectionHeader = &_sectionList[GetSectionCount() - 1];
	// Считаем обновленные размеры
	DWORD newSizeOfRawData = AlignUp(lastSectionHeader->SizeOfRawData + additionalSize, _peHeader->OptionalHeader.FileAlignment);
	DWORD newFileSize = (newSizeOfRawData - lastSectionHeader->SizeOfRawData) + _fileSize;
	DWORD newVirtualSize = lastSectionHeader->Misc.VirtualSize + additionalSize;
	DWORD newAlignedVirtualSize = AlignUp(newVirtualSize, _peHeader->OptionalHeader.SectionAlignment);
	DWORD newImageSize = AlignUp(_peHeader->OptionalHeader.SizeOfImage - AlignUp(lastSectionHeader->Misc.VirtualSize, _peHeader->OptionalHeader.SectionAlignment) + newVirtualSize, _peHeader->OptionalHeader.SectionAlignment);
	
	if( !Resize(newFileSize) )
	{
		return 0;
	}
	// После ресайза указатели стали недействительными
	lastSectionHeader = &_sectionList[GetSectionCount() - 1];
	// 
	if( lastSectionHeader->Characteristics & IMAGE_SCN_CNT_CODE)
	{
		_peHeader->OptionalHeader.SizeOfCode = newAlignedVirtualSize;
	}
	else if( lastSectionHeader->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
	{
		_peHeader->OptionalHeader.SizeOfInitializedData = newAlignedVirtualSize;
	}
	else if( lastSectionHeader->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA)
	{
		_peHeader->OptionalHeader.SizeOfUninitializedData = newAlignedVirtualSize;
	}
	
	_peHeader->OptionalHeader.SizeOfImage = newImageSize;
	lastSectionHeader->SizeOfRawData = newSizeOfRawData;
	DWORD oldVirtualsize = lastSectionHeader->Misc.VirtualSize;
	lastSectionHeader->Misc.VirtualSize = newVirtualSize;
	

	return lastSectionHeader->VirtualAddress + oldVirtualsize;
}

DWORD PEImagePatcher::WritePayload(const void* payload, DWORD payloadSize)
{
	DWORD payloadRVA = ResizeLastSection(payloadSize);
	DWORD payloadOffset = RvaToOffset(payloadRVA);
	if(payloadOffset == 0 || payloadOffset >=  _fileSize)
	{
		return 0; // Непонятная ошибка
	}
	// Собственнно, запишем payload
	memcpy(&_fileContent[payloadOffset], payload, payloadSize);
	SetLastSectionFlags(IMAGE_SCN_MEM_EXECUTE); // Установим флаг исполнения на последнюю секцию
	SetEnrtyPoint(payloadRVA);

	// DllCharacteristics влияют и на поведение простых exe файлов 
	// (у которых _не_ установлен флаг IMAGE_FILE_DLL в Characteristic)
	// Просто обнулим их нафик :)
	_peHeader->OptionalHeader.DllCharacteristics = 0;

	return payloadRVA;
}

DWORD PEImagePatcher::GetBaseOfImage()
{
	assert(_peHeader);
	return _peHeader->OptionalHeader.ImageBase;
}

DWORD PEImagePatcher::GetEntryPoint()
{
	assert(_peHeader);
	return _peHeader->OptionalHeader.AddressOfEntryPoint;
}

DWORD PEImagePatcher::GetPayloadRVA()
{
	PIMAGE_SECTION_HEADER lastSectionHeader = &_sectionList[GetSectionCount() - 1];
	return lastSectionHeader->VirtualAddress + lastSectionHeader->Misc.VirtualSize;
}

void PEImagePatcher::SetEnrtyPoint(DWORD ep)
{
	assert(_peHeader);
	_peHeader->OptionalHeader.AddressOfEntryPoint = ep;
}

DWORD PEImagePatcher::AlignUp(DWORD size, DWORD alignment)
{
	return ( size % alignment ) ? size + alignment - (size % alignment) : size;
}


PIMAGE_IMPORT_DESCRIPTOR PEImagePatcher::GetFirstImportDescriptor()
{
	DWORD importDescriptorOffset = RvaToOffset(_dataDirs[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	if(importDescriptorOffset == 0 || importDescriptorOffset >= _fileSize)
	{
		return NULL;
	}
	return reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(&_fileContent[importDescriptorOffset]);
}

PIMAGE_IMPORT_DESCRIPTOR PEImagePatcher::GetImportDescriptorByName(const char* dllName)
{
	DWORD importDescriptorCount = GetImportDescriprotCount();
	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = GetFirstImportDescriptor();
	for(DWORD i = 0; i != importDescriptorCount; i++)
	{
		DWORD nameOffset = RvaToOffset(importDescriptor[i].Name);
		if(!nameOffset || nameOffset >= _fileSize)
		{
			break;
		}
		const char* name = reinterpret_cast<const char*>(&_fileContent[nameOffset]);
		if(_stricmp(name, dllName) == 0)
		{
			return &importDescriptor[i];
		}
	}
	return NULL;
}

DWORD PEImagePatcher::GetImportDescriprotCount()
{
	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = GetFirstImportDescriptor();
	if(!importDescriptor)
	{
		return 0;
	}
	DWORD i = 0;
	do
	{
		if(!importDescriptor[i].Name || !importDescriptor[i].FirstThunk)
		{
			// Это конец таблицы
			break;
		}
		i++;
	} while(true);
	return i;
}

DWORD PEImagePatcher::GetImportTableThunk(const char* dllName, const char* functionName)
{
	PIMAGE_IMPORT_DESCRIPTOR dllImportDescriptor = GetImportDescriptorByName(dllName);
	if(!dllImportDescriptor)
	{
		return 0;
	}
	// Fucking Borland Workaround
	DWORD originalFirstThunkRVA = dllImportDescriptor->OriginalFirstThunk ? dllImportDescriptor->OriginalFirstThunk : dllImportDescriptor->FirstThunk;
	DWORD firstThunkRVA = dllImportDescriptor->FirstThunk;
	DWORD originalFirstThunkOffset = RvaToOffset(originalFirstThunkRVA);
	DWORD i = 0;
	do
	{
		DWORD thunkOffset = originalFirstThunkOffset + sizeof(IMAGE_THUNK_DATA)*i; // Бежим по массиву originalFirstThunk-ов
		DWORD thunkRVA = firstThunkRVA + sizeof(IMAGE_THUNK_DATA)*i; // Параллельно бежим по массиву firstThunk-ов
		if(thunkOffset == 0 || thunkOffset >= _fileSize)
		{
			return 0;
		}
		PIMAGE_THUNK_DATA thunkData = reinterpret_cast<PIMAGE_THUNK_DATA>(&_fileContent[thunkOffset]);
		if( !(thunkData->u1.AddressOfData) )
		{
			break; // Это последняя функция из Dll
		}
		if((thunkData->u1.AddressOfData) & IMAGE_ORDINAL_FLAG32)
		{
			i++;
			continue; // Эта функция импортируется через ординал
		}
		DWORD importByNameOffset = RvaToOffset(thunkData->u1.AddressOfData);
		if(importByNameOffset == 0 || importByNameOffset >= _fileSize)
		{
			return 0;
		}
		PIMAGE_IMPORT_BY_NAME importByName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(&_fileContent[importByNameOffset]);
		if( strcmp(reinterpret_cast<char*>(importByName->Name), functionName) == 0)
		{
			// Ok, нашли
			return thunkRVA;
		}
		i++;
	}
	while(true);
	return 0;
}

void PEImagePatcher::SetLastSectionFlags(DWORD flags)
{
	PIMAGE_SECTION_HEADER lastSectionHeader = &_sectionList[GetSectionCount() - 1];
	lastSectionHeader->Characteristics |= flags;
}

bool PEImagePatcher::CheckIfImportTableThunkCanBeAdded(const char* dllName)
{
	return GetImportDescriptorByName(dllName) == NULL;
}

void PEImagePatcher::AddImportTableThunks(const char* dllName, const char* functionName)
{
	if( !CheckIfImportTableThunkCanBeAdded(dllName) )
	{
		return ;
	}
	// Okay, для начала просто перенесем в другое место массив IMAGE_DATA_DSECRIPTOR'ов
	DWORD oldSizeOfImportTable = (GetImportDescriprotCount() + 1)*sizeof(IMAGE_IMPORT_DESCRIPTOR);

	// Считаем размер свободного места, которое нам необходимо для добавления 
	// записи об одной dll, которая импортирует одну функцию
	DWORD newSizeOfImportTabe = oldSizeOfImportTable + sizeof(IMAGE_IMPORT_DESCRIPTOR);
	DWORD sizeForDllName = strlen(dllName) + 1;
	DWORD sizeForFunctionName = strlen(functionName) + 1;
	DWORD sizeForImageThunksData = sizeof(IMAGE_THUNK_DATA)*4; // 1 для OriginalFirstThunk 1 для FirstThunk
	DWORD sizeForImportByNameStruct = sizeof(IMAGE_IMPORT_BY_NAME);

	DWORD additionalSize = newSizeOfImportTabe + sizeForDllName + sizeForFunctionName + sizeForImageThunksData + sizeForImportByNameStruct;

	// Выделяем место под новую таблицу импорта
	DWORD newImportTableRva = ResizeLastSection(additionalSize);

	// Получаем адрес старой
	void* oldImportTable = GetFirstImportDescriptor();
	// Скопируем таблицу импорта в новое место
	DWORD newImportTableOffset = RvaToOffset(newImportTableRva);
	if(!newImportTableOffset)
	{
		return ; // Непонятная ошибка
	}
	memset(&_fileContent[newImportTableOffset], 0, additionalSize); // Обнулим _всё_ пространство, куда будем писать данные
	memcpy(&_fileContent[newImportTableOffset], oldImportTable, oldSizeOfImportTable);

	// Обнулим старую таблицу
	memset(oldImportTable, 0, oldSizeOfImportTable);
	// Поменяем адрес в директории данных
	_dataDirs[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = newImportTableRva;
	_dataDirs[IMAGE_DIRECTORY_ENTRY_IMPORT].Size += sizeof(IMAGE_IMPORT_DESCRIPTOR);
	
	// Теперь добавляем новый элемент в таблицу

	PIMAGE_IMPORT_DESCRIPTOR newImportDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(&_fileContent[newImportTableOffset + oldSizeOfImportTable - sizeof(IMAGE_IMPORT_DESCRIPTOR)]);
	// Адресная арифметика в действии itt
	PIMAGE_THUNK_DATA imageThunkData1 = reinterpret_cast<PIMAGE_THUNK_DATA>(newImportDescriptor + 2); 
	PIMAGE_THUNK_DATA imageThunkData2 = imageThunkData1 + 2; // Один пустой чунк
	PIMAGE_IMPORT_BY_NAME importByName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(imageThunkData2 + 2); // Один пустой чунк

	char* functionNameInFile = reinterpret_cast<char*>(importByName->Name);
	char* dllNameInFile = functionNameInFile + sizeForFunctionName;

	// Заполним эти стуктуры данными
	// Начнем с имен dll и функции
	strcpy(dllNameInFile, dllName);
	strcpy(functionNameInFile, functionName);
	
	// Посчитаем RVA
	DWORD imageThunkData1RVA = newImportTableRva + newSizeOfImportTabe;
	DWORD imageThunkData2RVA = imageThunkData1RVA + 2*sizeof(IMAGE_THUNK_DATA);
	DWORD imageImportByNameRva = imageThunkData2RVA + 2*sizeof(IMAGE_THUNK_DATA);

	DWORD dllNameRVA = imageImportByNameRva + sizeof(IMAGE_IMPORT_BY_NAME) + sizeForFunctionName - 2;

	
	imageThunkData1->u1.AddressOfData = imageImportByNameRva;
	imageThunkData2->u1.AddressOfData = imageImportByNameRva;
	
	newImportDescriptor->FirstThunk = imageThunkData1RVA;
	newImportDescriptor->OriginalFirstThunk = imageThunkData2RVA;
	newImportDescriptor->Name = dllNameRVA;

	// Поставим на последнюю секцию аттрибут, разрешающий чтение и запись
	SetLastSectionFlags(IMAGE_SCN_MEM_WRITE);
	SetLastSectionFlags(IMAGE_SCN_MEM_READ);
	
	// На всякий случай уберем ссылку на IAT, если она есть
	_dataDirs[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = 0;
	_dataDirs[IMAGE_DIRECTORY_ENTRY_IAT].Size = 0;

	return ;
}
	
