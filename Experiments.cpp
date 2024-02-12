#include "StdAfx.h"
#include "Experiments.h"

CSimulationSpec g_SimulationSpec;
CCraneSpec g_CraneSpec;

CExperiments::CExperiments(void)
{
}

CExperiments::~CExperiments(void)
{
	//생성한 AHV 관련 CLASS를 삭제한다.
	if( m_pALVEmulator != NULL)	
		delete m_pALVEmulator;
	if( m_pProcessManager != NULL )
		delete m_pProcessManager;
	if( m_pEquipmentManager != NULL )
		delete m_pEquipmentManager;
}

void CExperiments::Initialize()
{
	bool experiments;
	//experiments = true;
	experiments = false;
	
	if(!experiments){
		//...simulation settings
		g_SimulationSpec.cargoType = CargoType_DoubleCycle;
		g_SimulationSpec.dispType = DR_GRASP;
		
		m_pALVEmulator		= new CALVEmulator();
		m_pEquipmentManager = new CALVManager();
		m_pProcessManager	= new CProcessManager();
		

		//초기화 순서는 꼭 지킬 것
		//m_pALVEmulator->SetNumberofVehicle( g_SimulationSpec.nVehicle );
		m_pEquipmentManager->InitializeManager( m_pProcessManager, m_pALVEmulator );
		m_pALVEmulator->InitializeEmulator( m_pEquipmentManager );
		m_pEquipmentManager->InitializeAgentInfo();
		m_pProcessManager->Initialize( m_pEquipmentManager );
	}
	else{		
		StartExperiments();
	}	
}

void CExperiments::StartExperiments()
{
	//시뮬레이션 배속 및 시간 설정
	g_SimulationSpec.delay = -1;		//최고속

	//시뮬레이션 설정 변수들
	EPrsType		processType[]	= { PrsType_Discharging, PrsType_Loading };	//시뮬레이션 타입
	int				randomSeeds[]	= { 3,  7, 13 };
	int				numberofVehicles[]	= { 8, 12, 16, 20, 24, 28, 32 };		//AHV의 수
	
	int p, n, r;
	for( p = 0; p < (int)sizeof(processType)/sizeof(EPrsType); ++p )
	{
		for( n = 0; n < (int)sizeof(numberofVehicles)/sizeof(int); ++n)
		{
			for( r = 0; r < (int)sizeof(randomSeeds)/sizeof(int); ++r)
			{
				/*
				g_YTSpec.simulation_nBay = 3;
				pProcess->prsType = processType[p];//양하, 적하
				g_YTSpec.nAHV = n;
				g_ApronSpec.randSEED = randomSeeds[r];
				*/
								
				m_pALVEmulator		 = new CALVEmulator();
				m_pProcessManager	 = new CProcessManager();
				m_pEquipmentManager  = new CALVManager();

				//초기화 순서는 꼭 지킬 것
				m_pEquipmentManager->InitializeManager( m_pProcessManager, m_pALVEmulator );
				m_pALVEmulator->InitializeEmulator( m_pEquipmentManager );
				m_pEquipmentManager->InitializeAgentInfo();
				m_pProcessManager->Initialize( m_pEquipmentManager );

				if( m_pALVEmulator != NULL)		delete m_pALVEmulator;
				if( m_pProcessManager != NULL )	delete m_pProcessManager;
				if( m_pEquipmentManager != NULL )	delete m_pEquipmentManager;
			}
		}		
	}	

	//요건 마지막에 윈도우 생성할 때 문제 생기는 것을 막기 위해서 - 완전 대충 5분만에 짠 Wrapper CLass임, 토달지말 것 !!!!
	m_pALVEmulator		 = new CALVEmulator();
	m_pProcessManager	 = new CProcessManager();
	m_pEquipmentManager  = new CALVManager();

	return;
}

