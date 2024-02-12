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

enum EHPAlign{				// TP ���� ����
	HPA_Horizontal,			// ���� ���� TP	(QC, ���� ��ġ��)
	HPA_Vertical			// ���� ���� TP	(���� ��ġ��)
};


class CHP
{
//friend CiTLC_AHVView;	//ATC ȭ�� ����� ���� ���� ���ʸ� ����Ѵ�

private:
	int			m_ID;						//TP ID
	string		m_StringID;

	//TP���¸� ��Ÿ���� ����
	//AHV Type�� AGV�� ��쿡�� m_bReserved�� ���
	bool		m_bReserved;				//AHV�� TP�� �����ϱ� ���Ͽ� ����
	bool		m_bHoisting;				//ũ������ TP�� �۾� ��

	int			m_Capacity;					//HP ���� �����̳� �뷮(?)
	int			m_MaximumCapacity;			//HP ���� ���� �����̳� �뷮(?)
	int			m_nContainer;
	CContainer	m_Container[2];

	
	EHPAlign	m_HPAlign;					//TP�� ����

	pair<int, int> m_unusableCrossLanes;	//������ ȸ���ݰ� ������ �����θ� ������ �� ����� �� ���� �������ε� 

	CPoint		m_ptCenter;					//�߽���ǥ
	CPoint		m_pt40ftContainerArea[4];	//40ft container�� ���� ���� ��ǥ
	CPoint		m_ptFront20ftContainerArea[4];	//����(�Ǵ� ����) �����̳ʰ� ���� ���� ��ǥ
	CPoint		m_ptBack20ftContainerArea[4];	//����(�Ǵ� ����) �����̳ʰ� ���� ���� ��ǥ

public:
	CHP(void);
	CHP( int id, CPoint& ptCenter, EHPAlign TPalign );
	~CHP(void);

	void	DrawTP(CDC& dc, double zoom);

	int		GetID(){ return m_ID; }
	void	SetStringID( string strID ){ m_StringID = strID; }
	string	GetStringID(){ return m_StringID; }

	int		GetnContainer()		{ return m_nContainer; }
	CPoint&	GetPtCenter()		{ return m_ptCenter; }				//TP�� �߽���ǥ ����
	int		GetMaximumCapacity(){ return m_MaximumCapacity; }
	string	GetPrsIDofContainer( int i ){ return m_Container[i].containerID; }//HP�� �����ִ� �����̳��� PrsID
	string	GetProcessID();

	//HP ���� ����
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
