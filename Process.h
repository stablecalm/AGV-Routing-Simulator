#pragma once

#include "ALVJob.h"
#include "StackLocation.h"

using namespace std;

class CProcess
{
public:
	string		prsID;			//���μ��� ID
	int			prsPriority;	//���μ��� �켱����
	EPrsType	prsType;		//���μ��� Ÿ��
	
	CStackLocation shipLoc;

	int		iQCID;
	int		iBlockID;

	string	qcID;
	string	BlockID;
	string	quayHP;
	string	yardHP;

	long		deadline;		//�۾��� ������� (���ϴ� �̸� ����)

	CALVJob* ALVJob;
	

public:
	CProcess(int processSequence, int qcID, EPrsType _prsType);
	~CProcess(void);	
	CProcess(const CProcess* prs);

	void SetBlockID( int block );
	void SetShipLocation(int height, int slot );
	void SetDeadline( long dline );
};
