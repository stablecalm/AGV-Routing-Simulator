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
	bool				m_bInitComplete;		//초기화 완료
	//CTimeSync			m_TimeSync;
	long				m_ClockTick;			//(1 clock = 0.1s)

	CALVEmulator*		m_pALVEmulator;
	CALVScheduler*		m_pALVScheduler;
	
	//...목록
	map<string, CALVJob*>	m_UnassignedJob;
	map<string, CALVJob*>	m_AssignedJob;

	vector<HPRsvInfo>	m_WaitQCPermission;		//(장비ID, 작업ID) QC의 HP 할당을 기다리는 작업 목록
	vector<HPRsvInfo>	m_WaitBlockPermission;	//(장비ID, 작업ID) Block의 HP 할당을 기다리는 작업 목록

	//...equipments
	map<string, CALVInfo*>	m_ALVInfo;

	//...vessel job
	bool				m_bVesselJobFinished;	//ProcessManager가 모든 Process를 배포했음을 알림. 즉 더이상 선석 작업이 발생하지 않음
	bool				m_bEmulationFinised;	//배포된 선석 작업이 모두 완료됨 - 시뮬레이션 종료

	//...Thread 관련 변수	
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
	CALVJobStatistics	m_ALVJobStatistics;		//통계 정보 처리기 (JobManager에 붙을 계획)

	//...equipments
	vector<CATC>		m_ATCs;
	vector<CQC>			m_QCs;

	map<string, int>	m_ATCIDList;
	map<string, int>	m_QCIDList;

	map<pair<string,string>, int>	m_TravelTimeMap;

private://method
	void GenerateCranes();												//QC와 ATC를 생성한다.
	void ReleaseHPNonDiAlogVersion( HPType hpType, string hpID );		//HP의 예약을 해제한다.

	void GenerateTravelTimeMap();										//ALV의 주행시간 Map을 만든다.
	
public://method
	void DoSingleIterationforCranes();									//QC와 ATC 시뮬레이션
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
