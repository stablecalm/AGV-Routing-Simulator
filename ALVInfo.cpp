#include "StdAfx.h"
#include "ALVInfo.h"

CALVInfo::CALVInfo(void)
{
	m_ID = "";
	m_JobState = JobState_Idle;
	m_ptVehicle.SetPoint(0,0);
	m_ALVJobList.clear();
	m_Capacity = 0;
	m_WPID = "LG0";
	
}

CALVInfo::CALVInfo(string ALVID, EALVJobState jobState, CPoint ptVehicle )
{
	m_ID = ALVID;
	m_JobState = jobState;
	m_ptVehicle = ptVehicle;
	return;
}

CALVInfo::~CALVInfo(void)
{
	m_ALVJobList.clear();
}

void CALVInfo::UpdateInfo( EALVJobState jobState, CPoint ptVehicle )
{
	m_JobState = jobState;
	m_ptVehicle = ptVehicle;	
	return;
}

void CALVInfo::AddJob(CALVJob* pALVJob)
{
	m_ALVJobList.push_back(pALVJob);	
}

void CALVInfo::DeleteJob(std::string jobID)
{
	deque<CALVJob*>::iterator indexJob;
	for( indexJob = m_ALVJobList.begin(); indexJob != m_ALVJobList.end(); ++indexJob ){
		if( jobID == (*indexJob)->jobID ){
			m_ALVJobList.erase(indexJob);
			return;
		}
	}
}

string CALVInfo::GetJobID()
{
	if( m_ALVJobList.empty() ){
		return "";
	}
	else{
		return m_ALVJobList[0]->jobID;
	}
}

EPrsType CALVInfo::GetJobType()
{
	if( m_ALVJobList.empty() ){
		return PrsType_Undefined;
	}
	else{
		return m_ALVJobList[0]->prsType;
	}
}

EALVJobState CALVInfo::GetJobState()
{
	if( m_ALVJobList.empty() ){
		return JobState_Undefined;
	}
	else{
		return m_ALVJobList[0]->GetJobState();
	}
}

bool CALVInfo::CheckJobExist()
{
	if( m_ALVJobList.empty()){
		return false;
	}
	return true;
}

int CALVInfo::GetCountofJob()
{
	if( m_ALVJobList.empty() ){
		return 0;
	}
	else{
		return (int)m_ALVJobList.size();
	}
}