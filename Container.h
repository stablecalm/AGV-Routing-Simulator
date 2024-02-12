#pragma once

using namespace std;

enum EContainerType{
	ConType_Undefined,
	ConType_General20ft,
	ConType_General40ft
};

class CContainer
{
public:
	CContainer(void);
public:
	~CContainer(void);
	
	EContainerType conType;
	string containerID;
	string groupID;
	//string containerType;
	string pod;
	string vesselID;
	string vesselName;


};
