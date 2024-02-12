#pragma once
#include "ALVJob.h"

using namespace std;

class CALVInfo
{
private:
	string			m_ID;			//���� ID
	EALVJobState	m_JobState;		//������ ���� �۾� ����
	CPoint			m_ptVehicle;	//���� ���� ��ġ
	deque<CALVJob*>	m_ALVJobList;	//ALV�� �Ҵ���� �۾� ���
	int				m_Capacity;		//ALV�� ��� ���� �����̳� ���� ���� 0:No Container, 1:One 20-ft Container, 2:Two 20-ft Containers or One 40-ft Container

	
public://for Estimate a Cost & dual-load
	string			m_WPID;			//������ ���������� ������ WPID;
	
public:
	CALVInfo(void);
	CALVInfo(string ALVID, EALVJobState jobState, CPoint ptVehicle );
	~CALVInfo(void);

	//�����̳� ��� ���� ����
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
