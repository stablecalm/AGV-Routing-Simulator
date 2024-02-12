#pragma once

#include "HP.h"
#include "Container.h"
#include "Process.h"
#include "ALVManager.h"

using namespace std;

class CALVManager;

class CQC
{
public:
	long			m_clockTick;	//qc 시간
private:
	//the number of available TP is same as the number of QC lane.
	int				m_ID;
	string			m_StringID;
		
	ETrolleyType	m_TrolleyType;	//QC의 트롤리 타입
	ESpreaderType	m_SpreaderType;	//QC의 스프레더 타입
	EBayType		m_BayType;		//QC가 현재 작업하는 베이의 종류
	ECargoType		m_CargoType;	//QC의 작업 - 양하,적하, 양하 후 적하, 더블 사이클링

	EDischargeDeadline m_DeadlineMethod;//수행 작업의 데드라인 결정 방법



	vector<CHP>		m_HPs;
	map<string, int> m_HPIDList;
	int				m_workingHP;	//QC가 작업하는 TP	

	CALVManager*	m_pEquipmentManager;

	deque<CProcess*>	m_DischargingSequence;	//양하 작업 순서
	deque<CProcess*>	m_LoadingSequence;		//적하 작업 순서

	CProcess*		m_pProcess;		//현재 작업 중인 프로세스, 또는 작업 중인 프로세스 목록의 첫번째 프로세스

	
	int				m_iThroughput;	//qc별 처리량
	int				m_idleTime;
	int				m_delayTime;//하나의 작업에 대한 IdleTime

	int				m_numberBayWorks;
	bool			m_worksDone;

	ECraneWorkState m_CWS;					//crane의 작업상태

	int			m_hoistingTimer;			//hoisting종료까지 남은 시간
	int			m_travelTimer;				//crane travel종료까지 남은 시간

	int			m_travelTime;
	int			m_hoistingTime;

	int			m_ConRowLength;				//컨테이너까지의 y축 길이
	int			m_ConColLength;				//컨테이너까지의 x축 길이

	double		m_travelMean;
	double		m_travelStdDev;

	//...QC 작업 수행 속도 관련
	double		m_trolleySpeed;
	double		m_emptyLoadSpeed;
	double		m_fullLoadSpeed;

	double		m_acctrolleySpeed;
	double		m_accemptyLoadSpeed;
	double		m_accfullLoadSpeed;

	//...for drawing
	CPoint		m_ptLocation;	//QC의 leftTop 좌표
	CPoint		m_ptTrolley;	//QC의 트롤리의 중심축 MostLeft 좌표
	int			m_IntervalOfMovement;	//for Drawing 한 번 수행시 트롤리의 이동간격

	//...statistics about double-cycling
	bool		m_PreviousDischarge;
	int			m_CountDoubleCycle;

public:
	CQC(void);
	CQC( ETrolleyType trolleyType, ESpreaderType spreaderType, int id, CPoint& initialPos );
	~CQC(void);

private://member
	void	DecideDeadlineofDischarge();	//양하 작업의 데드라인 계산

	void	DecideCurrentProcess();
	void	DoLoad();	//적하 작업 수행
	void	DoDischarge();	//양하 작업 수행

	int		GetHPofDischargingProcess(CProcess* pProcess);
	int		GetHPofLoadingProcess(CProcess* pProcess);

	void	HPReservationofProcess( int hpID );
	void	HPReleaseofProcess( int hpID );

	void	SetContainerofProcess( int hpID );
	void	ReleaseContainerofProcess( int hpID );

	void	SetDelayTime(int delayTime);

	void	DeclareProcessFinished( int hpID );	

	void	RecalculateDeadlineOfLoadingProcess();//적하 프로세스 데드라인 재설정

public:
	//drawing operation
	void	DrawHPs( CDC& dc, double dZoom);
	void	DrawCrane( CDC& dc, double zoom );

	void	Initialize( int id, int workingTP, CPoint& initialPos );
	void	DoSingleIteration();


	//...sets
	void	SetHP( CHP hp );
	void	SetDischargingSequence( deque<CProcess*> &prsSequence );
	void	SetLoadingSequence( deque<CProcess*> &prsSequence );
	void	SetEquipmentManager( CALVManager* pEM ){ m_pEquipmentManager = pEM; }
	void	SetBayType(EBayType bType ){ m_BayType = bType; }
	void	SetCargoType(ECargoType cType ){ m_CargoType = cType; }

	//...gets
	string	GetStringID(){ return m_StringID; }
	int		GetThroughput()		{ return m_iThroughput; }
	int		GetThroughputPerHour();
	int		GetIdleTime()		{ return m_idleTime; }
	int		GetnDoubleCycle()	{ return m_CountDoubleCycle; }
	ETrolleyType	GetTrolleyType(){ return m_TrolleyType; }
	ESpreaderType	GetSpreaderType(){ return m_SpreaderType; }
	EBayType		GetBayType(){ return m_BayType; }
	ECargoType		GetCargoType(){ return m_CargoType; }

	//...gets...추가된 변수(시간 관련)에 대한 처리	
	int		GetTravelTimer()	{ return m_travelTimer; }
	double	GetDistancefrom( CPoint& AHVPosition );	

	//Apron에서 호출
	int		GetReservableHPforPassPoint();
	int		GetReservableHPfor(int Capacity);

	int		CheckRemainHP();
	bool	CheckExistOnHP( string prsID );
	bool	CheckAdmissible( string jobID );
	bool	CheckAdmissibleLoad( string jobID );
	bool	CheckTopPriority( string jobID );
	bool	CheckKeepingSchedule(string jobID);
	bool	CheckReservable( string hpStringID, int needCapacity);

	//TP상태관련함수
	string	GetHPStringID( int hpID ){ return m_HPs[hpID].GetStringID(); }
	int		GetHPIntID( string hpID ){ return m_HPIDList[hpID]; }

	//...string형
	bool	SetHPReservation(string hpID){ return m_HPs[m_HPIDList[hpID]].SetReservation();}
	void	ReleaseHPReservation(string hpID){ m_HPs[m_HPIDList[hpID]].ReleaseReservation(); }

	void	SetHPHoisting(string hpID){ m_HPs[m_HPIDList[hpID]].SetHoisting()	;	}
	void	ReleaseHPHoisting(string hpID){ m_HPs[m_HPIDList[hpID]].ReleaseHoisting()	;   }

	void	SetHPContainer(string hpID, CContainer container ){ m_HPs[m_HPIDList[hpID]].SetContainer( container );	}	
	void	ReleaseHPContainer(string hpID, CContainer container){ m_HPs[m_HPIDList[hpID]].ReleaseContainer(container);	}

	bool	CheckHPReservation(string hpID){ return m_HPs[m_HPIDList[hpID]].CheckReservation()	;}
	bool	CheckHPHoisting(string hpID){ return m_HPs[m_HPIDList[hpID]].CheckHoisting()	;}	

	int		GetCurrentCapacityofHP(string hpID){ return m_HPs[m_HPIDList[hpID]].GetCurrentCapacity()	;}	
	int		GetRemainCapacityofHP(string hpID){ return m_HPs[m_HPIDList[hpID]].GetRemainCapacity()	;}	
	

	//...int형
	bool	SetHPReservation(int hpID)	{ return m_HPs[hpID].SetReservation(); }
	void	ReleaseHPReservation(int hpID){ m_HPs[hpID].ReleaseReservation(); }

	void	SetHPHoisting(int hpID)		{ m_HPs[hpID].SetHoisting()	;	}
	void	ReleaseHPHoisting(int hpID)	{ m_HPs[hpID].ReleaseHoisting()	;   }

	void	SetHPContainer(int hpID, CContainer container ){ m_HPs[hpID].SetContainer(container );	}	
	void	ReleaseHPContainer(int hpID, CContainer container)	{ m_HPs[hpID].ReleaseContainer(container);	}
		
	bool	CheckHPReservation(int hpID)	{ return m_HPs[hpID].CheckReservation()	;}
	bool	CheckHPHoisting(int hpID)		{ return m_HPs[hpID].CheckHoisting()	;}	

	int		GetCurrentCapacityofHP(int hpID)	{ return m_HPs[hpID].GetCurrentCapacity()	;}	
	int		GetRemainCapacityofHP(int hpID){ return m_HPs[hpID].GetRemainCapacity()	;}

	void	GetProcessIDList(deque<string>& jobIDList);

};
