// filetree2xml.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

// xerces includes
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>


#include <iostream>

XERCES_CPP_NAMESPACE_USE

	
void SaveFileTreeToXML(const XMLCh* RootPath, DOMNode* RootNode, XERCES_CPP_NAMESPACE::DOMDocument* Document)
{
	WIN32_FIND_DATA FindData;
	ZeroMemory( &FindData, sizeof(WIN32_FIND_DATA));
	XMLSize_t SearchPatternLen = XMLString::stringLen( RootPath ) + 1 +  1;
	XMLCh* SearchPattern = new XMLCh[SearchPatternLen];
	ZeroMemory(SearchPattern, sizeof(XMLCh)*SearchPatternLen );
	XMLString::catString( SearchPattern, RootPath );
	XMLString::catString( SearchPattern, L"*");
	
	HANDLE hSearch = FindFirstFile(SearchPattern, &FindData);
	delete[] SearchPattern;
	if( INVALID_HANDLE_VALUE == hSearch )
		return;
	do
	{
		XMLCh* FileName = FindData.cFileName;
		if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if( (XMLString::compareIString(FileName,L".")!=0) &&
				(XMLString::compareIString(FileName,L"..")!=0) )
			{
				DOMElement* dir = Document->createElement(L"directory");
				dir->setAttribute(L"name", FileName );
				RootNode->appendChild(dir);
				XMLSize_t SubDirPathLen = XMLString::stringLen( RootPath ) + XMLString::stringLen( FileName ) + 1  + 1;
				XMLCh* SubDir = new XMLCh[ SubDirPathLen ];
				ZeroMemory( SubDir, sizeof(XMLCh)*SubDirPathLen );
				XMLString::catString( SubDir, RootPath );
				XMLString::catString( SubDir, FileName);
				XMLString::catString( SubDir, L"\\" );
				SaveFileTreeToXML( SubDir, dir, Document );
				delete[] SubDir;
			}
		}
		else
		{
			DOMElement* file = Document->createElement(L"file");
			file->setAttribute(L"name", FileName );
			RootNode->appendChild(file);
		}
	}while( FindNextFile( hSearch, &FindData) != 0 );
	FindClose( hSearch );
}


void SaveXMLToFile(const XMLCh* FileName, XERCES_CPP_NAMESPACE::DOMDocument* Document )
{

}

void PrintXMLdoc(const XERCES_CPP_NAMESPACE::DOMDocument* doc)
{
	DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(L"Core");
	DOMLSSerializer   *theSerializer = ((DOMImplementationLS*)impl)->createLSSerializer();
    DOMLSOutput       *theOutputDesc = ((DOMImplementationLS*)impl)->createLSOutput();
	
	DOMConfiguration* serializerConfig=theSerializer->getDomConfig();
	if (serializerConfig->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
		serializerConfig->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

	XMLFormatTarget *outputStream = new StdOutFormatTarget();
	theOutputDesc->setByteStream(outputStream);

	theSerializer->write(doc, theOutputDesc);

    theOutputDesc->release();
    theSerializer->release();

	delete outputStream;
}

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch(...)
	{
		return 1;
	}

	XMLCh* Path  = NULL;
	DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(L"Core");
	XERCES_CPP_NAMESPACE::DOMDocument* doc = impl->createDocument( 0, L"directory", 0);
	
	if( argc >= 2)
		Path = argv[1];
	else
		Path = L".\\";
	SaveFileTreeToXML(Path, doc->getDocumentElement() ,doc);

	PrintXMLdoc( doc );

	doc->release();

	XMLPlatformUtils::Terminate();
	return 0;
}

