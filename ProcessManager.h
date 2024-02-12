#pragma once

#include "ALVManager.h"

using namespace std;

class CALVManager;

//Process를 생성하고 배포한다.
class CProcessManager
{
private:
	int	m_nTotalProcess;
	int	m_nFinishedProcess;
	int	m_processSequence;
	int	m_processPriority;
	int	m_LookaheadHorizon;
	int m_RedistributedCount;

	CALVManager*	m_pEquipmentManager;
	map<int, CProcess*> m_ProcessMap;
	vector<string>	m_UnscheduledALVJobList;//계획 대상에 포함되지 않았던 Process ID List;
	deque<CProcess*> m_QCProcessSequence[4];

private:
	//Process 생성
	CProcess*	GenerateProcess(int prsID, int qcID, EPrsType prsType, EContainerType conType);

	//Process 목록 생성
	void		MakeSequenceOfProcess();
	void		MakeSequenceOfSingleCycle();
	void		MakeSequenceOfDoubleCycle();

	//적하 시, ALV에게 프로세스 간 상대 위치를 알려주기 위하여 Process를 연결한다.
	void		CouplingProcessesforTwin(CProcess* prs1, CProcess* prs2);	//Twin-spreader 사용을 위하여 프로세스 연결
	void		CouplingProcessesforTandem(CProcess* prs1, CProcess* prs2);	//Tandem QC, 40ft 2개 연결
	void		CouplingProcessesforTandem(CProcess* prs1, CProcess* prs2, CProcess* prs3, CProcess* prs4);	//Tandem QC, 20ft 4개 연결

	//Process 배포
	void		DistributeProcesses();
public://constructor, destructor
	CProcessManager(void);
	~CProcessManager(void);

public://public operations
	void	Initialize( CALVManager* pEquipmentManager );
	void	ProcessFinished( CProcess* pProcess );
	CProcess* GetJob(int AHVID, int& priority, int designatedQCID = -1);
	CALVJob* GetALVJob(string jobID);

	void	DistributeALVJob();	

	//모든 Process에 접근이 가능한 ProcessManager에서 deadline 계산
	long	EstimateDeadline( int nSet, vector<string>& setHP, vector<string>& setALV, vector<string>& qcSchedule );
	string	GetQuayHPID( string prsID );
};
