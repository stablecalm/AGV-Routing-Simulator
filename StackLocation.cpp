#include "StdAfx.h"
#include ".\stacklocation.h"

CStackLocation::CStackLocation(void)
{
	bay = -1;
	slot = -1;
	height = -1;
}

CStackLocation::CStackLocation(int initBay, int initSlot, int initHeight)
{
	bay = initBay;
	slot = initSlot;
	height = initHeight;
}

CStackLocation::~CStackLocation(void)
{

}

bool operator==(const CStackLocation& lValue, const CStackLocation& rValue)
{
	return (lValue.bay == rValue.bay && lValue.bay == rValue.bay);
}