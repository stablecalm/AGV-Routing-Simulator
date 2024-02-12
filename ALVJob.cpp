#include "StdAfx.h"
#include "ALVJob.h"

HPRsvInfo::HPRsvInfo( string e, string c, string j, int n ){
	equipID = e;
	craneID = c;
	jobID = j;
	needCapacity = n;
}

HPRsvInfo::~HPRsvInfo()
{

}

bool HPRsvInfo::operator==(const HPRsvInfo& h2)
{
	return (this->equipID == h2.equipID && this->craneID == h2.craneID && this->jobID == h2.jobID );
}

CALVJob::CALVJob(void)
{
	//prsID = "";
	prsType = PrsType_Undefined;
	//jobID = "";
	assignedEquipID = "";
	jobState = JobState_Undefined;

	pattern = JP_Undefined;
		
	//QCID = "";
	QuayHPID	 = "";

	//BlockID = "";
	YardHPID	 = "";

	deadline = -1; //'-1' = INFINITE
	delayTime = 0;

	qcHPReservationComplete = false;
	blockHPReservationComplete = false;

	startTime = -1;
	travelToQuaysideBeginTime = -1;
	travelToQuaysideEndTime = -1;	
	waitQCPermissionBeginTime = -1;	
	waitQCPermissionEndTime = -1;			
	travelToQCBeginTime = -1;			
	travelToQCEndTime = -1;				
	transferAtQCBeginTime = -1;					
	transferAtQCEndTime = -1;				
	travelToYardsideBeginTime = -1;
	travelToYardsideEndTime = -1;
	waitBlockPermissionBeginTime = -1;	
	waitBlockPermissionEndTime = -1;		
	travelToBlockBeginTime = -1;			
	travelToBlockEndTime = -1;				
	transferAtBlockBeginTime = -1;				
	transferAtBlockEndTime = -1;			
	finishTime = -1;
}

CALVJob::CALVJob( const CALVJob* pALVJob)
{
	//통계 정보 처리를 위하여 필요한 함수

	jobID = pALVJob->jobID;
	assignedEquipID = pALVJob->assignedEquipID;
	prsID = pALVJob->prsID;
	prsType = pALVJob->prsType;
	shipBayID = pALVJob->shipBayID;
	container = pALVJob->container;

	pattern = pALVJob->pattern;

	deadline = pALVJob->deadline;
	delayTime = pALVJob->delayTime;

	//Pick-up, Drop-off 정보
	QCID = pALVJob->QCID;
	QuayHPID = pALVJob->QuayHPID;

	BlockID = pALVJob->BlockID;
	YardHPID = pALVJob->YardHPID;

	//HP 예약 관련 정보
	qcHPReservationComplete = pALVJob->qcHPReservationComplete;		//QC의 HP 예약 완료 여부
	blockHPReservationComplete = pALVJob->blockHPReservationComplete;		//Block의 HP 예약 완료 여부

	jobState = pALVJob->jobState;

	//연결 작업 리스트는 통계 정보 처리와 무관
	//ERelativePosition relativePosition;		//연결된 Job 고려 시 상대적 위치
	//vector<pair<ERelativePosition, string>> connectedJobList;

	//작업시간 정보
	startTime = pALVJob->startTime;
	travelToQuaysideBeginTime = pALVJob->travelToQuaysideBeginTime;		
	travelToQuaysideEndTime = pALVJob->travelToQuaysideEndTime;		
	waitQCPermissionBeginTime = pALVJob->waitQCPermissionBeginTime;		
	waitQCPermissionEndTime = pALVJob->waitQCPermissionEndTime;		
	travelToQCBeginTime = pALVJob->travelToQCBeginTime;			
	travelToQCEndTime = pALVJob->travelToQCEndTime;				
	transferAtQCBeginTime = pALVJob->transferAtQCBeginTime;			
	transferAtQCEndTime = pALVJob->transferAtQCEndTime;			
	travelToYardsideBeginTime = pALVJob->travelToYardsideBeginTime;		
	travelToYardsideEndTime = pALVJob->travelToYardsideEndTime;		
	waitBlockPermissionBeginTime = pALVJob->waitBlockPermissionBeginTime;	
	waitBlockPermissionEndTime = pALVJob->waitBlockPermissionEndTime;		
	travelToBlockBeginTime = pALVJob->travelToBlockBeginTime;			
	travelToBlockEndTime = pALVJob->travelToBlockEndTime;			
	transferAtBlockBeginTime = pALVJob->transferAtBlockBeginTime;		
	transferAtBlockEndTime = pALVJob->transferAtBlockEndTime;			
	finishTime = pALVJob->finishTime;						
}

CALVJob::~CALVJob(void)
{
}

bool CALVJob::CheckConnectedJob( string _jobID )
{
	ERelativePosition connectedJobPosition;
	vector<pair<ERelativePosition, string>>::iterator index;
	for( index = connectedJobList.begin(); index != connectedJobList.end(); ++index ){
		if( index->second == _jobID ){
			connectedJobPosition = index->first;
			break;
		}
	}
	if( index != connectedJobList.end() ){//연결된 경우
		if( relativePosition == ERP_Left && connectedJobPosition == ERP_Right ){
			return true;
		}
		else if( relativePosition == ERP_Right && connectedJobPosition == ERP_Left ){
			return true;
		}
		else if( relativePosition == ERP_LeftTop && connectedJobPosition == ERP_RightTop ){
			return true;
		}
		else if( relativePosition == ERP_RightTop && connectedJobPosition == ERP_LeftTop ){
			return true;
		}
		else if( relativePosition == ERP_LeftBottom && connectedJobPosition == ERP_RightBottom ){
			return true;
		}
		else if( relativePosition == ERP_RightBottom && connectedJobPosition == ERP_LeftBottom ){
			return true;
		}
	}
	
	return false;
}

void CALVJob::SetJobState( EALVJobState updatedJobState, long clockTick )
{
	if( jobState == JobState_Idle ){
		startTime = clockTick;
	}

	jobState = updatedJobState;

	//남은 작업 상태의 시간 초기화를 위하여 일부러 break를 걸지 않음
	if( prsType == PrsType_Discharging ){
		//아래는 양하 시 작업 순서
		switch(jobState)
		{
		case(JobState_TravelToQuayside):
			travelToQuaysideBeginTime = clockTick;
			travelToQuaysideEndTime = clockTick;			
		case(JobState_WaitQCPermission):
			waitQCPermissionBeginTime = clockTick;
			waitQCPermissionEndTime = clockTick;			
		case(JobState_TravelToDesignatedQC):
			travelToQCBeginTime = clockTick;			
			travelToQCEndTime = clockTick;
		case(JobState_TransferAtQC):
			transferAtQCBeginTime = clockTick;
			transferAtQCEndTime = clockTick;					
		case(JobState_TravelToYardside):
			travelToYardsideBeginTime = clockTick;
			travelToYardsideEndTime = clockTick;			
		case(JobState_WaitBlockPermission):
			waitBlockPermissionBeginTime = clockTick;
			waitBlockPermissionEndTime = clockTick;			
		case(JobState_TravelToDesignatedBlock):
			travelToBlockBeginTime = clockTick;
			travelToBlockEndTime = clockTick;				
		case(JobState_TransferAtBlock):
			transferAtBlockBeginTime = clockTick;
			transferAtBlockEndTime = clockTick;	
		case(JobState_Finished):
			finishTime = clockTick;
		default:			
			break;
		}

	}
	else if( prsType == PrsType_Loading ){
		//아래는 적하 시 작업 순서
		switch(jobState)
		{
		case(JobState_TravelToYardside):
			travelToYardsideBeginTime = clockTick;
			travelToYardsideEndTime = clockTick;			
		case(JobState_WaitBlockPermission):
			waitBlockPermissionBeginTime = clockTick;
			waitBlockPermissionEndTime = clockTick;			
		case(JobState_TravelToDesignatedBlock):
			travelToBlockBeginTime = clockTick;
			travelToBlockEndTime = clockTick;				
		case(JobState_TransferAtBlock):
			transferAtBlockBeginTime = clockTick;
			transferAtBlockEndTime = clockTick;	
		case(JobState_TravelToQuayside):
			travelToQuaysideBeginTime = clockTick;
			travelToQuaysideEndTime = clockTick;			
		case(JobState_WaitQCPermission):
			waitQCPermissionBeginTime = clockTick;
			waitQCPermissionEndTime = clockTick;			
		case(JobState_TravelToDesignatedQC):
			travelToQCBeginTime = clockTick;			
			travelToQCEndTime = clockTick;
		case(JobState_TransferAtQC):
			transferAtQCBeginTime = clockTick;
			transferAtQCEndTime = clockTick;		
		case(JobState_Finished):
			finishTime = clockTick;
		default:			
			break;
		}
	}
	return;
}

void CALVJob::SetEndTimeOf( EALVJobState jobState, long clockTick )
{
	//남은 작업 상태의 시간 초기화를 위하여 일부러 break를 걸지 않음
	
	//QCpermission과 BlockPermission의 EndTime은 절대 시간임
	//나머지 케이스는 Emulator에서 수행하는데 걸린 시간임
	bool firstUpdate = true;
	if( prsType == PrsType_Discharging ){
		//아래는 양하 시 작업 순서
		switch(jobState)
		{
		case(JobState_TravelToQuayside):
			if( firstUpdate ){
				clockTick += travelToQuaysideBeginTime;
				firstUpdate = false;
			}			
			travelToQuaysideEndTime = clockTick;			
			waitQCPermissionBeginTime = clockTick;
		case(JobState_WaitQCPermission):
			if( firstUpdate ){
				firstUpdate = false;
			}
			waitQCPermissionEndTime = clockTick;			
			travelToQCBeginTime = clockTick;
		case(JobState_TravelToDesignatedQC):
			if( firstUpdate ){
				clockTick += travelToQCBeginTime;
				firstUpdate = false;
			}
			travelToQCEndTime = clockTick;
			transferAtQCBeginTime = clockTick;
		case(JobState_TransferAtQC):
			if( firstUpdate ){
				clockTick += transferAtQCBeginTime;
				firstUpdate = false;
			}
			transferAtQCEndTime = clockTick;					
			travelToYardsideBeginTime = clockTick;
		case(JobState_TravelToYardside):
			if( firstUpdate ){
				clockTick += travelToYardsideBeginTime;
				firstUpdate = false;
			}
			travelToYardsideEndTime = clockTick;			
			waitBlockPermissionBeginTime = clockTick;
		case(JobState_WaitBlockPermission):			
			if( firstUpdate ){
				firstUpdate = false;
			}
			waitBlockPermissionEndTime = clockTick;			
			travelToBlockBeginTime = clockTick;
		case(JobState_TravelToDesignatedBlock):			
			if( firstUpdate ){
				clockTick += travelToBlockBeginTime;
				firstUpdate = false;
			}
			travelToBlockEndTime = clockTick;				
			transferAtBlockBeginTime = clockTick;
		case(JobState_TransferAtBlock):
			if( firstUpdate ){
				clockTick += transferAtBlockBeginTime;
				firstUpdate = false;
			}
			transferAtBlockEndTime = clockTick;	
		default:
			finishTime = clockTick;
			break;
		}

	}
	else if( prsType == PrsType_Loading ){
		//아래는 적하 시 작업 순서
		switch(jobState)
		{
		case(JobState_TravelToYardside):
			if( firstUpdate ){
				clockTick += travelToYardsideBeginTime;
				firstUpdate = false;
			}
			travelToYardsideEndTime = clockTick;			
			waitBlockPermissionBeginTime = clockTick;
		case(JobState_WaitBlockPermission):			
			if( firstUpdate ){
				firstUpdate = false;
			}
			waitBlockPermissionEndTime = clockTick;			
			travelToBlockBeginTime = clockTick;
		case(JobState_TravelToDesignatedBlock):			
			if( firstUpdate ){
				clockTick += travelToBlockBeginTime;
				firstUpdate = false;
			}
			travelToBlockEndTime = clockTick;				
			transferAtBlockBeginTime = clockTick;
		case(JobState_TransferAtBlock):
			if( firstUpdate ){
				clockTick += transferAtBlockBeginTime;
				firstUpdate = false;
			}
			transferAtBlockEndTime = clockTick;
			travelToQuaysideBeginTime = clockTick;
		case(JobState_TravelToQuayside):
			if( firstUpdate ){
				clockTick += travelToQuaysideBeginTime;
				firstUpdate = false;
			}			
			travelToQuaysideEndTime = clockTick;			
			waitQCPermissionBeginTime = clockTick;
		case(JobState_WaitQCPermission):
			if( firstUpdate ){
				firstUpdate = false;
			}
			waitQCPermissionEndTime = clockTick;			
			travelToQCBeginTime = clockTick;
		case(JobState_TravelToDesignatedQC):
			if( firstUpdate ){
				clockTick += travelToQCBeginTime;
				firstUpdate = false;
			}
			travelToQCEndTime = clockTick;
			transferAtQCBeginTime = clockTick;
		case(JobState_TransferAtQC):
			if( firstUpdate ){
				clockTick += transferAtQCBeginTime;
				firstUpdate = false;
			}
			transferAtQCEndTime = clockTick;					
			
		default:
			finishTime = clockTick;
			break;
		}
	}
	return;
}

void CALVJob::SetQCPermissionEndTime( long clockTick )
{
	waitQCPermissionEndTime = clockTick; 
	return;
}

void CALVJob::SetBlockPermissionEndTime( long clockTick )
{ 
	waitBlockPermissionEndTime = clockTick; 
	return;
}

void CALVJob::SetFinishTime( long clockTick )
{
	finishTime = clockTick; 
	return;
}

void CALVJob::SetElapsedTimeForTravelingToQuayside( long elapsedTime )
{
	travelToQuaysideEndTime = travelToQuaysideBeginTime + elapsedTime;
	return;
}

void CALVJob::SetElapsedTimeForTravelingToYardside( long elapsedTime )
{
	travelToYardsideEndTime = travelToQuaysideBeginTime + elapsedTime;
	return;
}

void CALVJob::SetElapsedTimeForTravelingToQC( long elapsedTime )
{
	travelToQCEndTime = travelToQCBeginTime + elapsedTime;
	return;
}

void CALVJob::SetElapsedTimeForTravelingToBlock( long elapsedTime )
{
	travelToBlockEndTime = travelToBlockBeginTime + elapsedTime;
	return;
}

void CALVJob::SetElapsedTimeToTransferAtQC( long elapsedTime )
{
	transferAtQCEndTime = transferAtQCBeginTime + elapsedTime;
	return;
}

void CALVJob::SetElapsedTimeToTransferAtBlock( long elapsedTime )
{
	transferAtBlockEndTime = transferAtBlockBeginTime + elapsedTime;
	return;
}

