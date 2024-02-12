#include "StdAfx.h"
#include "Experiments.h"

CSimulationSpec g_SimulationSpec;
CCraneSpec g_CraneSpec;

CExperiments::CExperiments(void)
{
}

CExperiments::~CExperiments(void)
{
	//������ AHV ���� CLASS�� �����Ѵ�.
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
		

		//�ʱ�ȭ ������ �� ��ų ��
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
	//�ùķ��̼� ��� �� �ð� ����
	g_SimulationSpec.delay = -1;		//�ְ��

	//�ùķ��̼� ���� ������
	EPrsType		processType[]	= { PrsType_Discharging, PrsType_Loading };	//�ùķ��̼� Ÿ��
	int				randomSeeds[]	= { 3,  7, 13 };
	int				numberofVehicles[]	= { 8, 12, 16, 20, 24, 28, 32 };		//AHV�� ��
	
	int p, n, r;
	for( p = 0; p < (int)sizeof(processType)/sizeof(EPrsType); ++p )
	{
		for( n = 0; n < (int)sizeof(numberofVehicles)/sizeof(int); ++n)
		{
			for( r = 0; r < (int)sizeof(randomSeeds)/sizeof(int); ++r)
			{
				/*
				g_YTSpec.simulation_nBay = 3;
				pProcess->prsType = processType[p];//����, ����
				g_YTSpec.nAHV = n;
				g_ApronSpec.randSEED = randomSeeds[r];
				*/
								
				m_pALVEmulator		 = new CALVEmulator();
				m_pProcessManager	 = new CProcessManager();
				m_pEquipmentManager  = new CALVManager();

				//�ʱ�ȭ ������ �� ��ų ��
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

	//��� �������� ������ ������ �� ���� ����� ���� ���� ���ؼ� - ���� ���� 5�и��� § Wrapper CLass��, ������� �� !!!!
	m_pALVEmulator		 = new CALVEmulator();
	m_pProcessManager	 = new CProcessManager();
	m_pEquipmentManager  = new CALVManager();

	return;
}

