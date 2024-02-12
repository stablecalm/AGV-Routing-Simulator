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
//friend CiTLC_AHVView;	//ATC ȭ�� ����� ���� ���� ���ʸ� ����Ѵ�
public:
	long			m_clockTick;
private:
	int				m_ID;
	string			m_StringID;

	vector<CHP>		m_HPs;
	map<string, int> m_HPIDList;
	
	int				m_iThroughput;				//atc�� ó����
	int				m_idleTime;

	CALVManager*	m_pEquipmentManager;

	CProcess*		m_pProcess;					//���� ó�� ���� �۾�

	vector<CProcess*>	m_DischargingSequence;		//���� �۾� ����
	vector<CProcess*>::iterator m_indexDischarge;	//���� �� ��� ���� ���� �۾��� ��ġ

	deque<CProcess*>	m_LoadingSeqPerQC[4];	//QC�� ���� �۾� ����
	vector<CProcess*>::iterator m_indexLoad;	//���� �� ��� ���� ���� �۾��� ��ġ

	deque<string>	m_RequestLoading;

	ECraneWorkState	m_CWS;						//ATC�� ���¸� ��Ÿ���� ����
	int				m_workingHP;				//���� �۾� ���� TP

	//crane travel �ð� ����
	int				m_travelTime;				
	double			m_travelMean;
	double			m_travelStdDev;

	//...Timer
	int				m_travelTimer;				//crane travel������� ���� �ð�
	int				m_hoistingTimer;			//hoisting������� ���� �ð�	

	//...for drawing
	CPoint			m_ptLocation;				//ũ����(���)�� ������ǥ, leftTop point�� ��ǥ.
	CPoint			m_ptTrolley;				//Ʈ�Ѹ��� �߽��� MostLeft ��ǥ
	CPoint			m_ptSpreader;
	int				m_xPosIntervalperDrawing;	//for Drawing �� �� ����� Ʈ�Ѹ��� �̵�����

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

	void	SetHP( CHP hp );	//HP �Ҵ�
	void	SetDischargingSequence( vector<CProcess*> &prsSequence );	//���� �۾� �Ҵ�
	void	SetLoadingSequence( vector<CProcess*> &prsSequence );		//���� �۾� �Ҵ�

	string	GetStringID(){ return m_StringID; }
	int		GetThroughput()		{ return m_iThroughput; }
	int		GetIdleTime()		{ return m_idleTime; }	
	CPoint&	GetPtLocation()				{ return m_ptLocation; }			//ũ������ ������ǥ�� ����
	
	void	SetEquipmentManager( CALVManager* pEM ){ m_pEquipmentManager = pEM; }

	//Apron���� ȣ��
	int		GetReservableHPforPassPoint();
	int		GetReservableHPfor(CContainer container);
	int		GetReservableHPfor(int Capacity);

	//TP���°����Լ�
	int		GetPairTP( int hpID );
	int		GetHPIntID( string hpID ){ return m_HPIDList[hpID]; }
	string	GetHPStringID( int hpID ){ return m_HPs[hpID].GetStringID(); }


	//string��
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
	
	//int��
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
