#pragma once
#include "ALVEnumeration.h"
#include "Container.h"
#include "ALVJob.h"

using namespace std;

enum HPType
{
	HPType_QuayHP,
	HPType_SeasideYardHP,
	HPType_LandsideYardHP,
	HPType_Undefined
};

enum EHPAlign{				// TP 정렬 방향
	HPA_Horizontal,			// 수평 방향 TP	(QC, 수평 장치장)
	HPA_Vertical			// 수직 방향 TP	(수직 장치장)
};


class CHP
{
//friend CiTLC_AHVView;	//ATC 화면 출력을 위해 내부 참초를 허용한다

private:
	int			m_ID;						//TP ID
	string		m_StringID;

	//TP상태를 나타내는 변수
	//AHV Type이 AGV인 경우에는 m_bReserved만 사용
	bool		m_bReserved;				//AHV가 TP에 진입하기 위하여 예약
	bool		m_bHoisting;				//크레인이 TP에 작업 중

	int			m_Capacity;					//HP 현재 컨테이너 용량(?)
	int			m_MaximumCapacity;			//HP 수용 가능 컨테이너 용량(?)
	int			m_nContainer;
	CContainer	m_Container[2];

	
	EHPAlign	m_HPAlign;					//TP의 방향

	pair<int, int> m_unusableCrossLanes;	//차량의 회전반경 때문에 주행경로를 구성할 때 사용할 수 없는 수직레인들 

	CPoint		m_ptCenter;					//중심좌표
	CPoint		m_pt40ftContainerArea[4];	//40ft container가 놓일 곳의 좌표
	CPoint		m_ptFront20ftContainerArea[4];	//좌측(또는 상측) 컨테이너가 놓일 곳의 좌표
	CPoint		m_ptBack20ftContainerArea[4];	//우측(또는 하측) 컨테이너가 놓일 곳의 좌표

public:
	CHP(void);
	CHP( int id, CPoint& ptCenter, EHPAlign TPalign );
	~CHP(void);

	void	DrawTP(CDC& dc, double zoom);

	int		GetID(){ return m_ID; }
	void	SetStringID( string strID ){ m_StringID = strID; }
	string	GetStringID(){ return m_StringID; }

	int		GetnContainer()		{ return m_nContainer; }
	CPoint&	GetPtCenter()		{ return m_ptCenter; }				//TP의 중심좌표 리턴
	int		GetMaximumCapacity(){ return m_MaximumCapacity; }
	string	GetPrsIDofContainer( int i ){ return m_Container[i].containerID; }//HP에 놓여있는 컨테이너의 PrsID
	string	GetProcessID();

	//HP 상태 관련
	void	SetHoisting()		{ m_bHoisting = true; }
	void	ReleaseHoisting()	{ m_bHoisting = false; }
	bool	CheckHoisting()		{ return m_bHoisting; }

	void	SetContainer( CContainer container );
	void	SetContainer( CContainer container, ERelativePosition rPosition );
	void	ReleaseContainer( CContainer container );
	int		GetCurrentCapacity(){ return m_Capacity;}
	int		GetRemainCapacity(){ return (m_MaximumCapacity-m_Capacity); }

	bool	SetReservation();
	void	ReleaseReservation(){ m_bReserved = false;}
	bool	CheckReservation()	{ return m_bReserved; }
	
};
