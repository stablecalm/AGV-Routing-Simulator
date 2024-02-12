#pragma once

using namespace std;

class CStackLocation
{
public:
	CStackLocation(void);
	CStackLocation(int initBay, int initStack, int initSlot);
	~CStackLocation(void);

public:
	int bay;
	int slot;
	int height;
};

bool operator==(const CStackLocation& lValue, const CStackLocation& rValue);