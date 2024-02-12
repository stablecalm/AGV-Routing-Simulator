#pragma once

#include "ALVJob.h"
#include "StackLocation.h"

using namespace std;

class CProcess
{
public:
	string		prsID;			//프로세스 ID
	int			prsPriority;	//프로세스 우선순위
	EPrsType	prsType;		//프로세스 타입
	
	CStackLocation shipLoc;

	int		iQCID;
	int		iBlockID;

	string	qcID;
	string	BlockID;
	string	quayHP;
	string	yardHP;

	long		deadline;		//작업의 데드라인 (적하는 미리 정의)

	CALVJob* ALVJob;
	

public:
	CProcess(int processSequence, int qcID, EPrsType _prsType);
	~CProcess(void);	
	CProcess(const CProcess* prs);

	void SetBlockID( int block );
	void SetShipLocation(int height, int slot );
	void SetDeadline( long dline );
};
