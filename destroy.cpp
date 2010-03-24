// destroy.cpp
#include <OrbitalIonCannon.h>
#include <Company.h>
#include <string>
#include <iostream>

int main(int argc, char* argv[])
{
	if( argc != 2 )
	{
		std::cout << "Usage: >destroy %companyname%" << std::cout;
		return 1;
	}
	std::string CompanyName = argv[1];
	
	CCompany Company;
	if( !Company.LoadInformation(CompanyName) )
	{
		std::cout << "Unknown company." << std::endl;
		return 1;
	}

	COrbitalIonCannon Cannon;

	if( !Cannon.Initialize() )
	{
		std::cout << "Unable to establish connectinon with orbital laser." << std::endl;
		return 1;
	}

	//typedef std::vector<CGeographicalObject> OfficeList_t;
	CCompany::OfficeList_t OfficeList;

	Company.GetOfficeList(OfficeList);

	for(CCompany::OfficeList_t::const_iterator It = OfficeList.begin();
		It != OfficeList.end();
		++It);
	{
		Cannon.MakeShoot( It->GetLatitude(), It->GetLongitude() );
	}

	return 0;
}




