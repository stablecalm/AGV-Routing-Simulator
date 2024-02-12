#pragma once

#include "CraneEnumeration.h"

#include "HP.h"
#include "Container.h"
#include "Process.h"
#include "ALVManager.h"

using namespace std;

class CALVManager;

class CATC
{
//friend CiTLC_AHVView;	//ATC 화면 출력을 위해 내부 참초를 허용한다
public:
	long			m_clockTick;
private:
	int				m_ID;
	string			m_StringID;

	vector<CHP>		m_HPs;
	map<string, int> m_HPIDList;
	
	int				m_iThroughput;				//atc별 처리량
	int				m_idleTime;

	CALVManager*	m_pEquipmentManager;

	CProcess*		m_pProcess;					//현재 처리 중인 작업

	vector<CProcess*>	m_DischargingSequence;		//양하 작업 순서
	vector<CProcess*>::iterator m_indexDischarge;	//양하 시 목록 상의 현재 작업의 위치

	deque<CProcess*>	m_LoadingSeqPerQC[4];	//QC별 적하 작업 순서
	vector<CProcess*>::iterator m_indexLoad;	//적하 시 목록 상의 현재 작업의 위치

	deque<string>	m_RequestLoading;

	ECraneWorkState	m_CWS;						//ATC의 상태를 나타내는 변수
	int				m_workingHP;				//현재 작업 중인 TP

	//crane travel 시간 관련
	int				m_travelTime;				
	double			m_travelMean;
	double			m_travelStdDev;

	//...Timer
	int				m_travelTimer;				//crane travel종료까지 남은 시간
	int				m_hoistingTimer;			//hoisting종료까지 남은 시간	

	//...for drawing
	CPoint			m_ptLocation;				//크레인(블록)의 기준좌표, leftTop point의 좌표.
	CPoint			m_ptTrolley;				//트롤리의 중심축 MostLeft 좌표
	CPoint			m_ptSpreader;
	int				m_xPosIntervalperDrawing;	//for Drawing 한 번 수행시 트롤리의 이동간격

public:
	CATC(void);
	CATC( int id, CPoint& initialPos );
	~CATC(void);

public://drawing operation
	void	DrawHPs( CDC& dc, double dZoom);
	void	DrawCrane( CDC& dc, double zoom  );

private://member
	int		EstimateTimeofSingleCycle();
	void	DecideCurrentProcess();
	int		DecideHPforLoadingContainer(CProcess *pProcess);

	CProcess* SelectDischargingProcess();
	CProcess* SelectLoadingProcess();

	void	DoLoad();
	void	DoDischarge();

public:
	void	DoSingleIteration();

	void	SetHP( CHP hp );	//HP 할당
	void	SetDischargingSequence( vector<CProcess*> &prsSequence );	//양하 작업 할당
	void	SetLoadingSequence( vector<CProcess*> &prsSequence );		//적하 작업 할당

	string	GetStringID(){ return m_StringID; }
	int		GetThroughput()		{ return m_iThroughput; }
	int		GetIdleTime()		{ return m_idleTime; }	
	CPoint&	GetPtLocation()				{ return m_ptLocation; }			//크레인의 기준좌표를 리턴
	
	void	SetEquipmentManager( CALVManager* pEM ){ m_pEquipmentManager = pEM; }

	//Apron에서 호출
	int		GetReservableHPforPassPoint();
	int		GetReservableHPfor(CContainer container);
	int		GetReservableHPfor(int Capacity);

	//TP상태관련함수
	int		GetPairTP( int hpID );
	int		GetHPIntID( string hpID ){ return m_HPIDList[hpID]; }
	string	GetHPStringID( int hpID ){ return m_HPs[hpID].GetStringID(); }


	//string형
	bool	SetHPReservation(string hpID){ return m_HPs[m_HPIDList[hpID]].SetReservation();}
	void	ReleaseHPReservation(string hpID){ m_HPs[m_HPIDList[hpID]].ReleaseReservation(); }

	void	SetHPHoisting(string hpID){ m_HPs[m_HPIDList[hpID]].SetHoisting()	;	}
	void	ReleaseHPHoisting(string hpID){ m_HPs[m_HPIDList[hpID]].ReleaseHoisting()	;   }

	void	SetHPContainer(string hpID, CContainer container){ m_HPs[m_HPIDList[hpID]].SetContainer( container );	}	
	void	ReleaseHPContainer(string hpID, CContainer container){ m_HPs[m_HPIDList[hpID]].ReleaseContainer(container);	}

	bool	CheckHPReservation(string hpID){ return m_HPs[m_HPIDList[hpID]].CheckReservation()	;}
	bool	CheckHPHoisting(string hpID){ return m_HPs[m_HPIDList[hpID]].CheckHoisting()	;}	

	int		GetCurrentCapacityofHP(string hpID){ return m_HPs[m_HPIDList[hpID]].GetCurrentCapacity()	;}
	int		GetRemainCapacityofHP(string hpID){ return m_HPs[m_HPIDList[hpID]].GetRemainCapacity()	;}	
	
	//int형
	bool	SetHPReservation(int hpID)	{ return m_HPs[hpID].SetReservation(); }
	void	ReleaseHPReservation(int hpID){ m_HPs[hpID].ReleaseReservation(); }

	void	SetHPHoisting(int hpID)		{ m_HPs[hpID].SetHoisting()	;	}
	void	ReleaseHPHoisting(int hpID)	{ m_HPs[hpID].ReleaseHoisting()	;   }

	void	SetHPContainer(int hpID, CContainer container )		{ m_HPs[hpID].SetContainer(container);	}	
	void	ReleaseHPContainer(int hpID, CContainer container)	{ m_HPs[hpID].ReleaseContainer(container);	}

	bool	CheckHPReservation(int hpID){ return m_HPs[hpID].CheckReservation()	;}
	bool	CheckHPHoisting(int hpID)		{ return m_HPs[hpID].CheckHoisting()	;}	

	int		GetCurrentCapacityofHP(int hpID)	{ return m_HPs[hpID].GetCurrentCapacity()	;}	
	int		GetRemainCapacityofHP(int hpID){ return m_HPs[hpID].GetRemainCapacity()	;}

	//for vehicles
	bool	CheckPairHPReservation(string hpID);
	bool	CheckPairHPReservation(int hpID);

	void	GetProcessIDList(deque<string>& jobIDList);	

};
