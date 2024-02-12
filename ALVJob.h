#pragma once

#include "Container.h"
#include "ALVEnumeration.h"

using namespace std;

enum ERelativePosition{	//���μ����� ����� ��ġ
	ERP_Undefined,
	ERP_Left,
	ERP_Right,
	ERP_Top,
	ERP_Bottom,
	ERP_LeftTop,
	ERP_LeftBottom,
	ERP_RightTop,
	ERP_RightBottom

};

enum EALVJobState
{
	JobState_Undefined,
	JobState_Idle,
	JobState_TravelToQuayside,
	JobState_WaitQCPermission,
	JobState_TravelToDesignatedQC,
	JobState_TransferAtQC,
	JobState_TravelToYardside,
	JobState_WaitBlockPermission,
	JobState_TravelToDesignatedBlock,
	JobState_TransferAtBlock,
	JobState_Finished,
	JobState_BackToWaitingArea

	//[����] AGV�� ��� ������ JobState�� �߰��Ǿ�� �Ѵ�.
	//JobState_WaitTransferAtQC,		//Quay HP�� ������ ���� QC�� �������� ������ ��ٸ� �ð�
	//JobState_WaitTransferAtBlock,		//Yard HP�� ������ ���� TC�� �������� ������ ��ٸ� �ð�
};

enum EJobPattern{
	JP_Undefined,
	JP_AANN,
	JP_ANNA,
	JP_ANAN,
	//addition for dual-load ALV
	JP_AIAI,
	JP_AIAN,
	JP_AINA,
	JP_ANAI
};


class HPRsvInfo
{
public:
	string equipID;
	string craneID;
	string jobID;
	int	needCapacity;

	HPRsvInfo(string,string,string,int);	
	~HPRsvInfo();
	bool operator == (const HPRsvInfo& h2);
};

class CALVJob
{
public://member
	string	jobID;
	string	assignedEquipID;
	string	prsID;
	EPrsType	prsType;
	int		shipBayID;
	CContainer container;

	EJobPattern	pattern;	//���ε� �� �۾� ���� ����
	
	//Pick-up, Drop-off ����
	string	QCID;
	string	QuayHPID;

	string	BlockID;
	string	YardHPID;

	//HP ���� ���� ����
	bool	qcHPReservationComplete;		//QC�� HP ���� �Ϸ� ����
	bool	blockHPReservationComplete;		//Block�� HP ���� �Ϸ� ����

	//������ ���� �� ������ ����ؾ���
	ERelativePosition relativePosition;		//����� Job ��� �� ����� ��ġ
	vector<pair<ERelativePosition, string>> connectedJobList;

	long	deadline;
	long	delayTime;	//�ش� �۾��� ���� �ð�(CProcess�� ����)
	
	EALVJobState	jobState;

	//�۾��ð� ����
	long	startTime;
	long	travelToQuaysideBeginTime;		//AHV�� ���� ������������ ������ ������ ����
	long	travelToQuaysideEndTime;		//AHV�� ���� ������������ ������ �Ϸ��� ����
	long	waitQCPermissionBeginTime;		//AHV�� ���� ���������� QC���� �����㰡�� ��û�� ����
	long	waitQCPermissionEndTime;		//AHV�� ���� ���������� QC���� �����㰡�� ���� ����
	long	travelToQCBeginTime;			//AHV�� ���� ���������� QC���� ������ ������ ����
	long	travelToQCEndTime;				//AHV�� ���� ���������� QC���� ������ �Ϸ��� ����
	long	transferAtQCBeginTime;			//AHV�� QC�� �����̳ʸ� �ְ� �޴� �۾��� ������ ����
	long	transferAtQCEndTime;			//AHV�� QC�� �����̳ʸ� �ְ� �޴� �۾��� �Ϸ��� ����
	long	travelToYardsideBeginTime;		//AHV�� ���� ������������ ������ ������ ����
	long	travelToYardsideEndTime;		//AHV�� ���� ������������ ������ �Ϸ��� ����
	long	waitBlockPermissionBeginTime;	//AHV�� QC���� Block������ �����㰡�� ��û�� ����
	long	waitBlockPermissionEndTime;		//AHV�� QC���� Block������ �����㰡�� ���� ����
	long	travelToBlockBeginTime;			//AHV�� QC���� Block������ ������ ������ ����
	long	travelToBlockEndTime;			//AHV�� QC���� Block������ ������ �Ϸ��� ����
	long	transferAtBlockBeginTime;		//AHV�� ATC�� �����̳ʸ� �ְ� �޴� �۾��� ������ ����
	long	transferAtBlockEndTime;			//AHV�� ATC�� �����̳ʸ� �ְ� �޴� �۾��� �Ϸ��� ����
	long	finishTime;						

	
	
public://method
	CALVJob(void);
	~CALVJob(void);
	CALVJob(const CALVJob* pALVJob);

	bool CheckConnectedJob( string jobID );

	//...sets
	void SetJobState( EALVJobState updatedJobState, long clockTick );
	void SetQCID(string qcID){ QCID = qcID; }
	void SetBlockID(string blockID){ BlockID = blockID; }
	void SetQuayHP( string hpID ){ QuayHPID = hpID; }
	void SetYardHP( string hpID ){ YardHPID = hpID; }
	
	void SetEndTimeOf( EALVJobState jobState, long clockTick );

	void SetQCPermissionEndTime( long clockTick );
	void SetBlockPermissionEndTime( long clockTick );
	void SetFinishTime( long clockTick );
	void SetElapsedTimeForTravelingToQuayside( long elapsedTime );
	void SetElapsedTimeForTravelingToYardside( long elapsedTime );
	void SetElapsedTimeForTravelingToQC( long elapsedTime );
	void SetElapsedTimeForTravelingToBlock( long elapsedTime );
	void SetElapsedTimeToTransferAtQC( long elapsedTime );
	void SetElapsedTimeToTransferAtBlock( long elapsedTime );

	//...gets
	string			GetJobID(){ return jobID; }
	string			GetAssignedEquipID(){ return assignedEquipID; }
	EPrsType		GetJobType(){ return prsType; }
	EALVJobState	GetJobState(){ return jobState; }

	string	GetQCID(){ return QCID; }
	string	GetBlockID(){ return BlockID; }
	string	GetQuayHP(){ return QuayHPID; }	
	string	GetYardHP(){ return YardHPID; }

	long	GetStartTime(){ return startTime; }
	long	GetTravelToQuaysideBeginTime()	{ return travelToQuaysideBeginTime; }
	long	GetTravelToQuaysideEndTime()	{ return travelToQuaysideEndTime;	}
	long	GetWaitQCPermissionBeginTime()	{ return waitQCPermissionBeginTime;	}
	long	GetWaitQCPermissionEndTime()	{ return waitQCPermissionEndTime;	}
	long	GetTravelToQCBeginTime()		{ return travelToQCBeginTime;		}
	long	GetTravelToQCEndTime()			{ return travelToQCEndTime;			}
	long	GetTransferAtQCBeginTime()		{ return transferAtQCBeginTime;		}
	long	GetTransferAtQCEndTime()		{ return transferAtQCEndTime;		}
	long	GetTravelToYardsideBeginTime()	{ return travelToYardsideBeginTime;	}
	long	GetTravelToYardsideEndTime()	{ return travelToYardsideEndTime;	}
	long	GetWaitBlockPermissionBeginTime(){return waitBlockPermissionBeginTime;}
	long	GetWaitBlockPermissionEndTime()	{ return waitBlockPermissionEndTime;}
	long	GetTravelToBlockBeginTime()		{ return travelToBlockBeginTime;	}
	long	GetTravelToBlockEndTime()		{ return travelToBlockEndTime;		}
	long	GetTransferAtBlockBeginTime()	{ return transferAtBlockBeginTime;	}
	long	GetTransferAtBlockEndTime()		{ return transferAtBlockEndTime;	}
	long	GetFinishTime(){ return finishTime; }
};
