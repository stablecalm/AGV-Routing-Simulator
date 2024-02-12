#include "StdAfx.h"
#include ".\process.h"

CProcess::CProcess(int processSequence, int _qcID, EPrsType _prsType)
{
	prsPriority = processSequence;
	prsType = _prsType;

	//Process 酒捞叼 积己
	ostringstream sout;
	if( prsType == PrsType_Discharging ){
		sout << "DP";	
	}
	else if( prsType == PrsType_Loading ){
		sout << "LP";
	}	
	if( processSequence < 10 ){
		sout << "00000";
	}
	else if( processSequence < 100 ){
		sout << "0000";
	}
	else if( processSequence < 1000 ){
		sout << "000";
	}
	else if( processSequence < 10000 ){
		sout << "00";
	}
	else if( processSequence < 100000){
		sout << "0";
	}
	sout << processSequence;

	prsID = sout.str();
	sout.str("");

	//QC ID 积己
	iQCID = _qcID;
	sout << "QC00E0" << _qcID;
	qcID = sout.str();
	sout.str("");

	//Block ID 积己
	iBlockID = rand() % g_SimulationSpec.nATC;
	sout << "YD00B0" << iBlockID;
	BlockID = sout.str();
	sout.str("");
	quayHP = "";
	yardHP = "";
	deadline = 864000;//24H
}

CProcess::~CProcess(void)
{
}

CProcess::CProcess( const CProcess* prs)
{
	prsID		= prs->prsID;
	prsPriority	= prs->prsPriority;		
	prsType		= prs->prsType;
	qcID		= prs->qcID;
	BlockID		= prs->BlockID;
	
	quayHP = prs->quayHP;
	yardHP = prs->yardHP;


	ALVJob->jobID							= prs->ALVJob->jobID;						
	ALVJob->assignedEquipID					= prs->ALVJob->assignedEquipID;			
	ALVJob->startTime						= prs->ALVJob->startTime;					
	//ALVJob->travelFromYardsideToWaitingAreaBeginTime		= prs->ALVJob->travelFromYardsideToWaitingAreaBeginTime;	
	//ALVJob->travelFromYardsideToWaitingAreaEndTime			= prs->ALVJob->travelFromYardsideToWaitingAreaEndTime;	
	ALVJob->waitQCPermissionBeginTime		= prs->ALVJob->waitQCPermissionBeginTime;	
	ALVJob->waitQCPermissionEndTime			= prs->ALVJob->waitQCPermissionEndTime;	
	ALVJob->travelToQCBeginTime				= prs->ALVJob->travelToQCBeginTime;		
	ALVJob->travelToQCEndTime				= prs->ALVJob->travelToQCEndTime;			
	ALVJob->transferAtQCBeginTime			= prs->ALVJob->transferAtQCBeginTime;		
	ALVJob->transferAtQCEndTime				= prs->ALVJob->transferAtQCEndTime;		
	ALVJob->waitBlockPermissionBeginTime		= prs->ALVJob->waitBlockPermissionBeginTime;
	ALVJob->waitBlockPermissionEndTime		= prs->ALVJob->waitBlockPermissionEndTime;	
	ALVJob->travelToBlockBeginTime			= prs->ALVJob->travelToBlockBeginTime;		
	ALVJob->travelToBlockEndTime				= prs->ALVJob->travelToBlockEndTime;		
	ALVJob->transferAtBlockBeginTime			= prs->ALVJob->transferAtBlockBeginTime;	
	ALVJob->transferAtBlockEndTime			= prs->ALVJob->transferAtBlockEndTime;		
	ALVJob->finishTime						= prs->ALVJob->finishTime;
	//ALVJob->idleDuration =	prs->ALVJob->idleDuration;	
	ALVJob->jobState				= prs->ALVJob->jobState;

	return;
}

void CProcess::SetBlockID( int block )
{
	iBlockID = block;

	ostringstream sout;
	sout << "YD00B0" << iBlockID;
	BlockID = sout.str();
	sout.str("");

	if( this->ALVJob != NULL ){
		this->ALVJob->BlockID = BlockID;
	}

	return;
}

void CProcess::SetDeadline( long dline )
{
	deadline = dline;
	if( ALVJob != NULL ){
		ALVJob->deadline = dline;
	}
	return;
}

void CProcess::SetShipLocation(int height, int slot )
{
	shipLoc.height	= height;
	shipLoc.slot	= slot;
	return;
}