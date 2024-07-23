#ifndef __eleIDMap__
#define __eleIDMap__

#include<iostream>
#include<map>

class eleIDMap
{

public:
	std::map<std::string, UInt_t> eleIDmap;

	eleIDMap()
	{

		eleIDmap["fiducial"]          = 0x0001;

		eleIDmap["vetoRun2022"]             = 0x0002;
                eleIDmap["looseRun2022"]            = 0x0004;
                eleIDmap["mediumRun2022"]           = 0x0008;
                eleIDmap["tightRun2022"]            = 0x0010;

	}

};

#endif


