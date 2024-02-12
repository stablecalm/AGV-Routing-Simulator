#pragma once

//header file of equipments
#include "YTEmulator\ALVEmulator.h"
#include "ALVInfo.h"
#include "ALVScheduler.h"

//for ver.Lofty-Hound Only
#include "ALVJobStatistics.h"
#include "ATC.h"
#include "QC.h"
#include "ProcessManager.h"
#include "HP.h"

#define	 GETCRANEID(x) atoi(x.substr(6,7).c_str())

using namespace std;

class CProcessManager;
class CALVEmulator;
class CALVScheduler;
class CATC;
class CQC;

class CALVManager
{
//from DiALog::ALVManager
private://member	
	string				m_ID;
	bool				m_bInitComplete;		//�ʱ�ȭ �Ϸ�
	//CTimeSync			m_TimeSync;
	long				m_ClockTick;			//(1 clock = 0.1s)

	CALVEmulator*		m_pALVEmulator;
	CALVScheduler*		m_pALVScheduler;
	
	//...���
	map<string, CALVJob*>	m_UnassignedJob;
	map<string, CALVJob*>	m_AssignedJob;

	vector<HPRsvInfo>	m_WaitQCPermission;		//(���ID, �۾�ID) QC�� HP �Ҵ��� ��ٸ��� �۾� ���
	vector<HPRsvInfo>	m_WaitBlockPermission;	//(���ID, �۾�ID) Block�� HP �Ҵ��� ��ٸ��� �۾� ���

	//...equipments
	map<string, CALVInfo*>	m_ALVInfo;

	//...vessel job
	bool				m_bVesselJobFinished;	//ProcessManager�� ��� Process�� ���������� �˸�. �� ���̻� ���� �۾��� �߻����� ����
	bool				m_bEmulationFinised;	//������ ���� �۾��� ��� �Ϸ�� - �ùķ��̼� ����

	//...Thread ���� ����	
	HANDLE				m_hThreadSimulation;
	unsigned int		m_ThreadSimulationID;
	bool				m_ThreadTerminate;

private://methods
	//traffic control
	//...sub controls
	void HandleRequestDispatching();
	void HandleRequestQCPermission();
	void HandleRequestBlockPermission();
	
public://methods
	CALVManager(void);
	~CALVManager(void);

	//...drawing operation
	void DrawHPs( CDC& dc, double dZoom );
	void DrawCranes( CDC& dc, double dZoom );

	//thread function
	static unsigned int _stdcall ThreadProcSimulation(void* pArguments);
	void NonThreadSimulation();

	//from DiALog::ALVManager
	void InitializeManager( CProcessManager* pProcessManager, CALVEmulator* pALVEmulator );
	void InitializeAgentInfo();

	void AddNewALVJobSequence( deque<CALVJob*> jobSequence );
	
	//...related to ALVEmulator
	void InitializeALVInformation(string ID, CPoint ptVehicle);
	void UpdateALVInfo( string equipID, string jobID, CPoint pt, string wpID );
	void UpdateALVJobState( string jobID, EALVJobState jobStateOperating );
	void UpdateALVJobInfo( string jobID, long elapsedTime, string hpID = "" );
	void DeclareJobFinished( string equipID, string jobID );

	//...related to ALVScheduler
	int  GetDistanceBetween( string equipID, HPType hpType, string craneID );

	void RequestHPReservation( HPType hpType, HPRsvInfo rsvInfo );
	void ConfirmHPReservation( HPType hpType, HPRsvInfo rsvInfo );
	void ReleaseHP( HPType hpType, string hpID );
	bool CheckBlockHPReserved( string blockID, string hpID );

	//simulation
	void DoSingleIteration();

	void AssignJob(string equipID, CALVJob* pALVJob);

	//run and stop thread
	void StartSimulation();
	void PauseSimulation();
	void FinishSimulation();

	void InformVesselJobFinished();
	void InformEmulationFinished(){ m_bEmulationFinised = true; }

	//...gets
	bool	CheckInitComplete(){ return m_bInitComplete; }
	string	GetID(){ return m_ID; }
	long	GetClockTick(){ return m_ClockTick; }

	//...statistics
	void	WriteResult();
	

//////////////////////////////////////////////////////////////////////////
//for ver.Lofty_Hound Only
//////////////////////////////////////////////////////////////////////////
private://member
	CProcessManager*	m_pProcessManager;
	CALVJobStatistics	m_ALVJobStatistics;		//��� ���� ó���� (JobManager�� ���� ��ȹ)

	//...equipments
	vector<CATC>		m_ATCs;
	vector<CQC>			m_QCs;

	map<string, int>	m_ATCIDList;
	map<string, int>	m_QCIDList;

	map<pair<string,string>, int>	m_TravelTimeMap;

private://method
	void GenerateCranes();												//QC�� ATC�� �����Ѵ�.
	void ReleaseHPNonDiAlogVersion( HPType hpType, string hpID );		//HP�� ������ �����Ѵ�.

	void GenerateTravelTimeMap();										//ALV�� ����ð� Map�� �����.
	
public://method
	void DoSingleIterationforCranes();									//QC�� ATC �ùķ��̼�
	ETrolleyType	GetQCTrolleyType( int qcID );
	ESpreaderType	GetQCSpreaderType( int qcID );
	EBayType		GetQCBayType(int qcID );
	ECargoType		GetQCCargoType(int qcID );

	int	 GetIdleTimeOfQC( int qcID );
	void SetProcessSequence( EPrsType prsType, EEquipType equipType, int equipID, deque<CProcess*> &prsSequence );
	void SetProcessSequence( EPrsType prsType, EEquipType equipType, int equipID, vector<CProcess*> &prsSequence );
	void ProcessFinished(CProcess* prs);
	long EstimateDeadline( string qcID, int nSet, vector<string>& setHP, vector<string>& qcSchedule );

	int	 GetTravelTimefromMap( string sourceID, string destinationID );
	int  GetTravelTimeEstimated( string equipID, CALVJob* job );

	bool CheckQCSchedule(CProcess* pProcess);
	bool CheckQCSchedule(CALVJob* pALVJob);

	bool CheckTopoPriority( string qcID, string obID);
	bool CheckKeepingSchedule(CALVJob* pALVJob);

	void SetCraneSchedule( int qc, deque<CProcess*> craneSchedule);

	void GetProcessIDListOfQC( int qc, deque<string>& jobIDList );
	void GetProcessIDListOfATC( int atc, deque<string>& jobIDList );
	
};
