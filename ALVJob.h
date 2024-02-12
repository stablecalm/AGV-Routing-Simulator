#pragma once

#include "Container.h"
#include "ALVEnumeration.h"

using namespace std;

enum ERelativePosition{	//프로세스의 상대적 위치
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

	//[참고] AGV의 경우 다음의 JobState가 추가되어야 한다.
	//JobState_WaitTransferAtQC,		//Quay HP에 도착한 다음 QC가 서비스해줄 때까지 기다린 시간
	//JobState_WaitTransferAtBlock,		//Yard HP에 도착한 다음 TC가 서비스해줄 때까지 기다린 시간
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

	EJobPattern	pattern;	//듀얼로드 시 작업 수행 패턴
	
	//Pick-up, Drop-off 정보
	string	QCID;
	string	QuayHPID;

	string	BlockID;
	string	YardHPID;

	//HP 예약 관련 정보
	bool	qcHPReservationComplete;		//QC의 HP 예약 완료 여부
	bool	blockHPReservationComplete;		//Block의 HP 예약 완료 여부

	//차량은 적하 시 다음을 고려해야함
	ERelativePosition relativePosition;		//연결된 Job 고려 시 상대적 위치
	vector<pair<ERelativePosition, string>> connectedJobList;

	long	deadline;
	long	delayTime;	//해당 작업의 지연 시간(CProcess의 정보)
	
	EALVJobState	jobState;

	//작업시간 정보
	long	startTime;
	long	travelToQuaysideBeginTime;		//AHV가 해측 정지선으로의 주행을 시작한 시점
	long	travelToQuaysideEndTime;		//AHV가 해측 정지선으로의 주행을 완료한 시점
	long	waitQCPermissionBeginTime;		//AHV가 해측 정지선에서 QC로의 진입허가를 요청한 시점
	long	waitQCPermissionEndTime;		//AHV가 해측 정지선에서 QC로의 진입허가를 받은 시점
	long	travelToQCBeginTime;			//AHV가 해측 정지선에서 QC로의 주행을 시작한 시점
	long	travelToQCEndTime;				//AHV가 해측 정지선에서 QC로의 주행을 완료한 시점
	long	transferAtQCBeginTime;			//AHV가 QC와 컨테이너를 주고 받는 작업을 시작한 시점
	long	transferAtQCEndTime;			//AHV가 QC와 컨테이너를 주고 받는 작업을 완료한 시점
	long	travelToYardsideBeginTime;		//AHV가 해측 정지선으로의 주행을 시작한 시점
	long	travelToYardsideEndTime;		//AHV가 해측 정지선으로의 주행을 완료한 시점
	long	waitBlockPermissionBeginTime;	//AHV가 QC에서 Block으로의 진입허가를 요청한 시점
	long	waitBlockPermissionEndTime;		//AHV가 QC에서 Block으로의 진입허가를 받은 시점
	long	travelToBlockBeginTime;			//AHV가 QC에서 Block으로의 주행을 시작한 시점
	long	travelToBlockEndTime;			//AHV가 QC에서 Block으로의 주행을 완료한 시점
	long	transferAtBlockBeginTime;		//AHV가 ATC와 컨테이너를 주고 받는 작업을 시작한 시점
	long	transferAtBlockEndTime;			//AHV가 ATC와 컨테이너를 주고 받는 작업을 완료한 시점
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
