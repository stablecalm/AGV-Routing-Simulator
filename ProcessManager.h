#pragma once

#include "ALVManager.h"

using namespace std;

class CALVManager;

//Process�� �����ϰ� �����Ѵ�.
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
	vector<string>	m_UnscheduledALVJobList;//��ȹ ��� ���Ե��� �ʾҴ� Process ID List;
	deque<CProcess*> m_QCProcessSequence[4];

private:
	//Process ����
	CProcess*	GenerateProcess(int prsID, int qcID, EPrsType prsType, EContainerType conType);

	//Process ��� ����
	void		MakeSequenceOfProcess();
	void		MakeSequenceOfSingleCycle();
	void		MakeSequenceOfDoubleCycle();

	//���� ��, ALV���� ���μ��� �� ��� ��ġ�� �˷��ֱ� ���Ͽ� Process�� �����Ѵ�.
	void		CouplingProcessesforTwin(CProcess* prs1, CProcess* prs2);	//Twin-spreader ����� ���Ͽ� ���μ��� ����
	void		CouplingProcessesforTandem(CProcess* prs1, CProcess* prs2);	//Tandem QC, 40ft 2�� ����
	void		CouplingProcessesforTandem(CProcess* prs1, CProcess* prs2, CProcess* prs3, CProcess* prs4);	//Tandem QC, 20ft 4�� ����

	//Process ����
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

	//��� Process�� ������ ������ ProcessManager���� deadline ���
	long	EstimateDeadline( int nSet, vector<string>& setHP, vector<string>& setALV, vector<string>& qcSchedule );
	string	GetQuayHPID( string prsID );
};
