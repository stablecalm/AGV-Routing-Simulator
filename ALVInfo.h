#pragma once
#include "ALVJob.h"

using namespace std;

class CALVInfo
{
private:
	string			m_ID;			//차량 ID
	EALVJobState	m_JobState;		//차량의 현재 작업 상태
	CPoint			m_ptVehicle;	//차량 현재 위치
	deque<CALVJob*>	m_ALVJobList;	//ALV가 할당받은 작업 목록
	int				m_Capacity;		//ALV가 운반 중인 컨테이너 양적 정보 0:No Container, 1:One 20-ft Container, 2:Two 20-ft Containers or One 40-ft Container

	
public://for Estimate a Cost & dual-load
	string			m_WPID;			//차량이 마지막으로 점유한 WPID;
	
public:
	CALVInfo(void);
	CALVInfo(string ALVID, EALVJobState jobState, CPoint ptVehicle );
	~CALVInfo(void);

	//컨테이너 취급 정보 관련
	void	Load40ftContainer(){ m_Capacity += 2;}
	void	Load20ftContainer(){ m_Capacity += 1; }
	void	Unload40ftContainer(){ m_Capacity -= 2; }
	void	Unload20ftContainer(){ m_Capacity -= 1; }

	void	UpdateInfo( EALVJobState jobState, CPoint ptVehicle );
	void	AddJob(CALVJob* pALVJob);
	void	DeleteJob(string jobID);

	void	SetJobState( EALVJobState jobState ){ m_JobState = jobState; }
	void	SetPointOfVehicle( CPoint &ptVehicle ){ m_ptVehicle = ptVehicle; }
	void	SetPointOfVehicle( int xPos, int yPos ){ m_ptVehicle.SetPoint( xPos, yPos ); }
	
	bool	CheckJobExist();
	int		GetCountofJob();

	string			GetID(){ return m_ID; }
	CPoint			GetCurrentPoint(){ return m_ptVehicle; }
	CALVJob*		GetCurrentJob(){ return m_ALVJobList[0]; }
	CALVJob*		GetSecondJob(){ return m_ALVJobList[1]; }
	string			GetJobID();
	EPrsType		GetJobType();
	EALVJobState	GetJobState();
	int				GetCapacity();
	
	
};
