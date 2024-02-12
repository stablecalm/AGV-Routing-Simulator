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
	long			m_clockTick;	//qc �ð�
private:
	//the number of available TP is same as the number of QC lane.
	int				m_ID;
	string			m_StringID;
		
	ETrolleyType	m_TrolleyType;	//QC�� Ʈ�Ѹ� Ÿ��
	ESpreaderType	m_SpreaderType;	//QC�� �������� Ÿ��
	EBayType		m_BayType;		//QC�� ���� �۾��ϴ� ������ ����
	ECargoType		m_CargoType;	//QC�� �۾� - ����,����, ���� �� ����, ���� ����Ŭ��

	EDischargeDeadline m_DeadlineMethod;//���� �۾��� ������� ���� ���



	vector<CHP>		m_HPs;
	map<string, int> m_HPIDList;
	int				m_workingHP;	//QC�� �۾��ϴ� TP	

	CALVManager*	m_pEquipmentManager;

	deque<CProcess*>	m_DischargingSequence;	//���� �۾� ����
	deque<CProcess*>	m_LoadingSequence;		//���� �۾� ����

	CProcess*		m_pProcess;		//���� �۾� ���� ���μ���, �Ǵ� �۾� ���� ���μ��� ����� ù��° ���μ���

	
	int				m_iThroughput;	//qc�� ó����
	int				m_idleTime;
	int				m_delayTime;//�ϳ��� �۾��� ���� IdleTime

	int				m_numberBayWorks;
	bool			m_worksDone;

	ECraneWorkState m_CWS;					//crane�� �۾�����

	int			m_hoistingTimer;			//hoisting������� ���� �ð�
	int			m_travelTimer;				//crane travel������� ���� �ð�

	int			m_travelTime;
	int			m_hoistingTime;

	int			m_ConRowLength;				//�����̳ʱ����� y�� ����
	int			m_ConColLength;				//�����̳ʱ����� x�� ����

	double		m_travelMean;
	double		m_travelStdDev;

	//...QC �۾� ���� �ӵ� ����
	double		m_trolleySpeed;
	double		m_emptyLoadSpeed;
	double		m_fullLoadSpeed;

	double		m_acctrolleySpeed;
	double		m_accemptyLoadSpeed;
	double		m_accfullLoadSpeed;

	//...for drawing
	CPoint		m_ptLocation;	//QC�� leftTop ��ǥ
	CPoint		m_ptTrolley;	//QC�� Ʈ�Ѹ��� �߽��� MostLeft ��ǥ
	int			m_IntervalOfMovement;	//for Drawing �� �� ����� Ʈ�Ѹ��� �̵�����

	//...statistics about double-cycling
	bool		m_PreviousDischarge;
	int			m_CountDoubleCycle;

public:
	CQC(void);
	CQC( ETrolleyType trolleyType, ESpreaderType spreaderType, int id, CPoint& initialPos );
	~CQC(void);

private://member
	void	DecideDeadlineofDischarge();	//���� �۾��� ������� ���

	void	DecideCurrentProcess();
	void	DoLoad();	//���� �۾� ����
	void	DoDischarge();	//���� �۾� ����

	int		GetHPofDischargingProcess(CProcess* pProcess);
	int		GetHPofLoadingProcess(CProcess* pProcess);

	void	HPReservationofProcess( int hpID );
	void	HPReleaseofProcess( int hpID );

	void	SetContainerofProcess( int hpID );
	void	ReleaseContainerofProcess( int hpID );

	void	SetDelayTime(int delayTime);

	void	DeclareProcessFinished( int hpID );	

	void	RecalculateDeadlineOfLoadingProcess();//���� ���μ��� ������� �缳��

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

	//...gets...�߰��� ����(�ð� ����)�� ���� ó��	
	int		GetTravelTimer()	{ return m_travelTimer; }
	double	GetDistancefrom( CPoint& AHVPosition );	

	//Apron���� ȣ��
	int		GetReservableHPforPassPoint();
	int		GetReservableHPfor(int Capacity);

	int		CheckRemainHP();
	bool	CheckExistOnHP( string prsID );
	bool	CheckAdmissible( string jobID );
	bool	CheckAdmissibleLoad( string jobID );
	bool	CheckTopPriority( string jobID );
	bool	CheckKeepingSchedule(string jobID);
	bool	CheckReservable( string hpStringID, int needCapacity);

	//TP���°����Լ�
	string	GetHPStringID( int hpID ){ return m_HPs[hpID].GetStringID(); }
	int		GetHPIntID( string hpID ){ return m_HPIDList[hpID]; }

	//...string��
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
	

	//...int��
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
