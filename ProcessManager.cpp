#include "StdAfx.h"
#include ".\processmanager.h"

#define GETPROCESSID(x) atoi((x).substr(2,8).c_str())

//constructor, destructor----------------------------------------------------------------------
CProcessManager::CProcessManager(void)
:m_processSequence(0), m_processPriority(0), m_nFinishedProcess(0), 
m_LookaheadHorizon(12), m_RedistributedCount(4)
{
}

CProcessManager::~CProcessManager(void)
{
	for(int i = 0; i < 4; ++i){
		m_QCProcessSequence[i].clear();
	}	

	//생성한 CProcess와 CALVJob을 모두 삭제한다.
	if( m_ProcessMap.empty() == false ){
		CProcess* pProcess;
		map<int, CProcess*>::iterator iterProcess;
		for( iterProcess = m_ProcessMap.begin(); iterProcess != m_ProcessMap.end(); ++iterProcess ){
			pProcess = iterProcess->second;
			if(pProcess != NULL ){
				CALVJob* pALVJob = pProcess->ALVJob;
				if( pALVJob != NULL ){
					delete pALVJob;
				}
				delete pProcess;
			}		
		}
	}
	
	m_ProcessMap.clear();
	
}

//private operations----------------------------------------------------------------------------
CProcess* CProcessManager::GenerateProcess(int prsID, int _qcID, EPrsType prsType, EContainerType conType )
{
	CProcess* prs = new CProcess( prsID, _qcID, prsType);

	//ALVJob 정보 설정
	CALVJob* ALVJob = new CALVJob();
	ALVJob->jobID = prs->prsID;
	ALVJob->prsType = prs->prsType;
	ALVJob->container.containerID = prs->prsID;
	ALVJob->container.conType = conType;
	ALVJob->QCID = prs->qcID;
	ALVJob->BlockID = prs->BlockID;
	ALVJob->jobState = JobState_Idle;
	ALVJob->deadline = prs->deadline;
	prs->ALVJob = ALVJob;

	return prs;
}
/*
void CProcessManager::MakeSequenceOfProcess()
{
	//각 크레인 당 총 베이 수는 2개로 고정
	//한 베이는 25개의 stack으로 되어있으며, 한 stack은 10 row로 구성되어있음
	//single spreader type의 crane은 한 번에 한 베이를 처리 
	//twin spreader type의 crane은 한 번에 베이 두 개를 처리
	if( g_SimulationSpec.cargoType == CargoType_Discharge || g_SimulationSpec.cargoType == CargoType_Load || g_SimulationSpec.cargoType == CargoType_Mixed ){
		MakeSequenceOfSingleCycle();
	}
	else if( g_SimulationSpec.cargoType == CargoType_DoubleCycle ){
		MakeSequenceOfDoubleCycle();
	}
	return;
}
*/
void CProcessManager::MakeSequenceOfProcess()
{
	int prsID = 0;//프로세스 ID
	int row, col;
	int seqCol[] = {1,25,2,24,3,23,4,22,5,21,6,20,7,19,8,18,9,17,10,16,11,15,12,14,13};

	ETrolleyType	tType;
	ESpreaderType	sType;
	EBayType		bType;

	int qc;
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		tType = m_pEquipmentManager->GetQCTrolleyType(qc);
		sType = m_pEquipmentManager->GetQCSpreaderType(qc);
		bType = m_pEquipmentManager->GetQCBayType(qc);

		if( sType == Spreader_Single ){
			if( m_pEquipmentManager->GetQCCargoType(qc) == CargoType_Discharge ){
				for( col = 0; col < 25; ++col){
					for( row = 10; row > 0; --row ){
						CProcess* prs;
						if( bType == BayType_20ft ){									
							prs = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							prs->SetShipLocation(row,seqCol[col]);
							m_ProcessMap.insert( make_pair(prsID, prs) );
							++prsID;
						}
						else if( bType == BayType_40ft ){
							prs = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General40ft );
							prs->SetShipLocation(row, seqCol[col]);
							m_ProcessMap.insert( make_pair(prsID, prs) );
							++prsID;
						}							
					}
				}
			}
			else if( m_pEquipmentManager->GetQCCargoType(qc) == CargoType_Load ){
				long count = 1;
				for( col = 0; col < 25; ++col){
					for( row = 1; row <= 10; ++row ){
						CProcess* prs;
						if( bType == BayType_20ft ){
							prs = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							prs->SetShipLocation( row, seqCol[col] );
							prs->SetDeadline((long)(count * g_SimulationSpec.halfCycletime *2));
							m_ProcessMap.insert( make_pair(prsID, prs) );
							++prsID;
						}
						else if( bType == BayType_40ft ){
							prs = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General40ft );
							prs->SetShipLocation(row, seqCol[col]);
							prs->SetDeadline((long)(count * g_SimulationSpec.halfCycletime *2));
							m_ProcessMap.insert( make_pair(prsID, prs) );
							++prsID;
						}
						++count;
					}
				}
			}
			else if( m_pEquipmentManager->GetQCCargoType(qc) == CargoType_DoubleCycle ){
				long count = 1;
				CProcess *prsD, *prsL;
				int rowLoad, rowDischarge;
				for( col = 0; col < 24; col += 2){
					rowLoad = 1;
					rowDischarge = 10;
					for( row = 0; row < 10; ++row ){
						if( bType == BayType_20ft ){
							prsD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							prsD->SetShipLocation(rowDischarge,seqCol[col]);
							prsD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, prsD) );
							++prsID;
							prsL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							prsL->SetShipLocation(rowLoad,seqCol[col+1]);
							prsL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, prsL) );
							++prsID;
						}
						else if( bType == BayType_40ft ){
							prsD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General40ft );
							prsD->SetShipLocation(rowDischarge,seqCol[col]);
							prsD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, prsD) );
							prsID += g_SimulationSpec.nQC;
							prsL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General40ft );
							prsL->SetShipLocation(rowLoad,seqCol[col+1]);
							prsL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, prsL) );
							++prsID;
						}
						++rowLoad;
						--rowDischarge;
						++count;
					}

				}
			}
		}
		else if( sType == Spreader_Twin ){
			if( m_pEquipmentManager->GetQCCargoType(qc) == CargoType_Discharge ){
				for( col = 0; col < 25; ++col){
					for( row = 10; row > 0; --row ){
						CProcess *prs, *connectedPrs;
						if( bType == BayType_20ft ){//20ft x 20ft 물량 생성 - 양하 시 목적 블록의 위치는 동일하다.						
							prs = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							prs->SetShipLocation(row,seqCol[col]);
							m_ProcessMap.insert( make_pair(prsID, prs) );
							++prsID;

							connectedPrs = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							connectedPrs->SetShipLocation(row,seqCol[col]);
							connectedPrs->SetBlockID(prs->iBlockID);
							m_ProcessMap.insert( make_pair(prsID, connectedPrs) );
							++prsID;
							CouplingProcessesforTwin(prs, connectedPrs);
						}
						else if( bType == BayType_40ft ){
							prs = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General40ft );
							prs->SetShipLocation(row, seqCol[col]);
							m_ProcessMap.insert( make_pair(prsID, prs) );
							++prsID;
						}							
					}
				}
			}
			else if( m_pEquipmentManager->GetQCCargoType(qc) == CargoType_Load ){
				long count = 1;
				for( col = 0; col < 25; ++col){
					for( row = 1; row <= 10; ++row ){
						CProcess *prs, *connectedPrs;
						if( bType == BayType_20ft ){//20ft x 20ft 물량 생성
							prs = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							prs->SetShipLocation( row, seqCol[col] );
							prs->SetDeadline((long)(count * g_SimulationSpec.halfCycletime *2));
							m_ProcessMap.insert( make_pair(prsID, prs) );
							++prsID;

							connectedPrs = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							connectedPrs->SetShipLocation(row,seqCol[col]);
							connectedPrs->SetDeadline((long)(count * g_SimulationSpec.halfCycletime *2));
							m_ProcessMap.insert( make_pair(prsID, connectedPrs) );
							++prsID;
							CouplingProcessesforTwin(prs, connectedPrs);
						}
						else if( bType == BayType_40ft ){
							prs = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General40ft );
							prs->SetShipLocation(row, seqCol[col]);
							prs->SetDeadline((long)(count * g_SimulationSpec.halfCycletime *2));
							m_ProcessMap.insert( make_pair(prsID, prs) );
							++prsID;							
						}
						++count;
					}
				}						

			}
			else if( m_pEquipmentManager->GetQCCargoType(qc) == CargoType_DoubleCycle ){
				long count = 1;
				CProcess *prsD, *connectedD, *prsL, *connectedL;
				int rowLoad, rowDischarge;
				for( col = 0; col < 24; col += 2){
					rowLoad = 1;
					rowDischarge = 10;
					for( row = 0; row < 10; ++row ){
						if( bType == BayType_20ft ){
							prsD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							prsD->SetShipLocation(rowDischarge,seqCol[col]);
							prsD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, prsD) );
							++prsID;

							connectedD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							connectedD->SetShipLocation(rowDischarge,seqCol[col]);
							connectedD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							connectedD->SetBlockID(prsD->iBlockID);
							m_ProcessMap.insert( make_pair(prsID, connectedD) );
							++prsID;

							CouplingProcessesforTwin(prsD, connectedD);

							prsL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							prsL->SetShipLocation(rowLoad,seqCol[col+1]);
							prsL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, prsL) );
							++prsID;

							connectedL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							connectedL->SetShipLocation(rowLoad,seqCol[col+1]);
							connectedL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, connectedL) );
							++prsID;

							CouplingProcessesforTwin(prsL, connectedL);
						}
						else if( bType == BayType_40ft ){
							prsD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General40ft );
							prsD->SetShipLocation(rowDischarge,seqCol[col]);
							prsD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, prsD) );
							++prsID;

							prsL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General40ft );
							prsL->SetShipLocation(rowLoad,seqCol[col+1]);
							prsL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, prsL) );
							++prsID;
						}
						++rowLoad;
						--rowDischarge;
						++count;
					}

				}
			}
		}
	}

	return;
}

void CProcessManager::MakeSequenceOfDoubleCycle()
{
	int prsID = 0;
	int row, col;
	int seqCol[] = {1,25,2,24,3,23,4,22,5,21,6,20,7,19,8,18,9,17,10,16,11,15,12,14,13};
	
	ETrolleyType	tType;
	ESpreaderType	sType;
	EBayType		bType;

	int qc;
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		tType = m_pEquipmentManager->GetQCTrolleyType(qc);
		sType = m_pEquipmentManager->GetQCSpreaderType(qc);
		bType = m_pEquipmentManager->GetQCBayType(qc);

		if( sType == Spreader_Single ){
			if( m_pEquipmentManager->GetQCCargoType(qc) == CargoType_DoubleCycle ){
				long count = 1;
				CProcess *prsD, *prsL;
				int rowLoad, rowDischarge;
				for( col = 0; col < 24; col += 2){
					rowLoad = 1;
					rowDischarge = 10;
					for( row = 0; row < 10; ++row ){
						if( bType == BayType_20ft ){
							prsD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							prsD->SetShipLocation(rowDischarge,seqCol[col]);
							prsD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, prsD) );
							++prsID;
							prsL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							prsL->SetShipLocation(rowLoad,seqCol[col+1]);
							prsL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, prsL) );
							++prsID;
						}
						else if( bType == BayType_40ft ){
							prsD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General40ft );
							prsD->SetShipLocation(rowDischarge,seqCol[col]);
							prsD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, prsD) );
							prsID += g_SimulationSpec.nQC;
							prsL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General40ft );
							prsL->SetShipLocation(rowLoad,seqCol[col+1]);
							prsL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, prsL) );
							++prsID;
						}
						++rowLoad;
						--rowDischarge;
						++count;
					}
					
				}

			}
		}
		else if( sType == Spreader_Twin ){
			if( m_pEquipmentManager->GetQCCargoType(qc) == CargoType_DoubleCycle ){
				long count = 1;
				CProcess *prsD, *connectedD, *prsL, *connectedL;
				int rowLoad, rowDischarge;
				for( col = 0; col < 24; col += 2){
					rowLoad = 1;
					rowDischarge = 10;
					for( row = 0; row < 10; ++row ){
						if( bType == BayType_20ft ){
							prsD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							prsD->SetShipLocation(rowDischarge,seqCol[col]);
							prsD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, prsD) );
							++prsID;

							connectedD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General20ft );
							connectedD->SetShipLocation(rowDischarge,seqCol[col]);
							connectedD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, connectedD) );
							++prsID;

							CouplingProcessesforTwin(prsD, connectedD);

							prsL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							prsL->SetShipLocation(rowLoad,seqCol[col+1]);
							prsL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, prsL) );
							++prsID;

							connectedL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General20ft );
							connectedL->SetShipLocation(rowLoad,seqCol[col+1]);
							connectedL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, connectedL) );
							++prsID;

							CouplingProcessesforTwin(prsL, connectedL);
						}
						else if( bType == BayType_40ft ){
							prsD = GenerateProcess( prsID, qc, PrsType_Discharging, ConType_General40ft );
							prsD->SetShipLocation(rowDischarge,seqCol[col]);
							prsD->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2) );
							m_ProcessMap.insert( make_pair(prsID, prsD) );
							++prsID;

							prsL = GenerateProcess( prsID, qc, PrsType_Loading, ConType_General40ft );
							prsL->SetShipLocation(rowLoad,seqCol[col+1]);
							prsL->SetDeadline( (long)(count * g_SimulationSpec.halfCycletime *2 + g_SimulationSpec.halfCycletime) );
							m_ProcessMap.insert( make_pair(prsID, prsL) );
							++prsID;
						}
						++rowLoad;
						--rowDischarge;
						++count;
					}

				}

			}
		}

	}//end of for

	return;
}

void CProcessManager::CouplingProcessesforTwin(CProcess* prs1, CProcess* prs2)
{
	//for CProcess 1
	prs1->ALVJob->relativePosition = ERP_Left;
	prs1->ALVJob->connectedJobList.push_back( make_pair(ERP_Right, prs2->prsID));

	//for CProcess 2
	prs2->ALVJob->relativePosition = ERP_Right;
	prs2->ALVJob->connectedJobList.push_back( make_pair(ERP_Left, prs1->prsID));
	return;
}

void CProcessManager::CouplingProcessesforTandem(CProcess* prs1, CProcess* prs2)
{
	//for CProcess 1
	prs1->ALVJob->relativePosition = ERP_Top;
	prs1->ALVJob->connectedJobList.push_back( make_pair(ERP_Bottom, prs2->prsID));

	//for CProcess 2
	prs2->ALVJob->relativePosition = ERP_Bottom;
	prs2->ALVJob->connectedJobList.push_back( make_pair(ERP_Top, prs1->prsID));
	return;
}

void CProcessManager::CouplingProcessesforTandem(CProcess* prs1, CProcess* prs2, CProcess* prs3, CProcess* prs4)
{
	//The simple and best way -ㅂ-
	//for CProcess 1
	prs1->ALVJob->relativePosition = ERP_LeftTop;
	prs1->ALVJob->connectedJobList.push_back( make_pair(ERP_RightTop, prs2->prsID));
	prs1->ALVJob->connectedJobList.push_back( make_pair(ERP_LeftBottom, prs3->prsID));
	prs1->ALVJob->connectedJobList.push_back( make_pair(ERP_RightBottom, prs4->prsID));

	//for CProcess 2
	prs2->ALVJob->connectedJobList.push_back( make_pair(ERP_LeftTop, prs1->prsID));
	prs2->ALVJob->relativePosition = ERP_RightTop;
	prs2->ALVJob->connectedJobList.push_back( make_pair(ERP_LeftBottom, prs3->prsID));
	prs2->ALVJob->connectedJobList.push_back( make_pair(ERP_RightBottom, prs4->prsID));

	//for CProcess 3
	prs3->ALVJob->connectedJobList.push_back( make_pair(ERP_LeftTop, prs1->prsID));
	prs3->ALVJob->connectedJobList.push_back( make_pair(ERP_RightTop, prs2->prsID));
	prs3->ALVJob->relativePosition = ERP_LeftBottom;
	prs3->ALVJob->connectedJobList.push_back( make_pair(ERP_RightBottom, prs4->prsID));

	//for CProcess 4
	prs4->ALVJob->connectedJobList.push_back( make_pair(ERP_LeftTop, prs1->prsID));
	prs4->ALVJob->connectedJobList.push_back( make_pair(ERP_RightTop, prs2->prsID));
	prs4->ALVJob->connectedJobList.push_back( make_pair(ERP_LeftBottom, prs3->prsID));
	prs4->ALVJob->relativePosition = ERP_RightBottom;
	return;
}


void CProcessManager::DistributeProcesses()
{
	map<int, CProcess*>::iterator itrSequence;

	//...assign processes to quay cranes
	deque<CProcess*> loadingSequenceOfQC;
	deque<CProcess*> dischargingSequenceOfQC;
	int qc;	
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){		
		for( itrSequence = m_ProcessMap.begin(); itrSequence != m_ProcessMap.end(); ++itrSequence ){
			CProcess* process = (*itrSequence).second;
			if( process->iQCID == qc ){
				m_QCProcessSequence[qc].push_back(process);		//ALV 배포용 프로세스 목록 생성
				if( process->prsType == PrsType_Discharging ){
					dischargingSequenceOfQC.push_back( process );
				}
				else if( process->prsType == PrsType_Loading ){
					loadingSequenceOfQC.push_back( process );
				}				
			}			
		}

		m_pEquipmentManager->SetProcessSequence( PrsType_Discharging, EquipType_QC, qc, dischargingSequenceOfQC );		
		m_pEquipmentManager->SetProcessSequence( PrsType_Loading, EquipType_QC, qc, loadingSequenceOfQC );		
		loadingSequenceOfQC.clear();
		dischargingSequenceOfQC.clear();
	}

	//...assign processes to block
	vector<CProcess*> loadingSequenceOfBlock;
	vector<CProcess*> dischargingSequenceOfBlock;
	int block;	
	for( block = 0; block < g_SimulationSpec.nATC; ++block ){		
		for( itrSequence = m_ProcessMap.begin(); itrSequence != m_ProcessMap.end(); ++itrSequence ){
			CProcess* process = (*itrSequence).second;
			if( process->iBlockID == block ){
				if( process->prsType == PrsType_Discharging ){
					dischargingSequenceOfBlock.push_back( process );
				}
				else if( process->prsType == PrsType_Loading ){
					loadingSequenceOfBlock.push_back( process );
				}				
			}			 
		}
		m_pEquipmentManager->SetProcessSequence( PrsType_Discharging, EquipType_ATC, block, dischargingSequenceOfBlock );
		m_pEquipmentManager->SetProcessSequence( PrsType_Loading, EquipType_ATC, block, loadingSequenceOfBlock );
		loadingSequenceOfBlock.clear();
		dischargingSequenceOfBlock.clear();
	}	
	
	//...make ALVJob List;
	for( itrSequence = m_ProcessMap.begin(); itrSequence != m_ProcessMap.end(); ++itrSequence ){
		CProcess* process = (*itrSequence).second;
		m_UnscheduledALVJobList.push_back(process->prsID);
	}
	

	return;
}

void CProcessManager::DistributeALVJob()
{
	if( m_UnscheduledALVJobList.empty() ){
		m_pEquipmentManager->InformVesselJobFinished();
	}
	else{
		deque<string> jobIDList;	
		int i;
		//먼저 QC의 HP에 놓여있는 양하 작업의 ID와 HP에 없는 첫번째 적하 작업의 ID를 모은다..
		for( i = 0; i < g_SimulationSpec.nQC; ++i ){
			m_pEquipmentManager->GetProcessIDListOfQC( i, jobIDList );
		}
		/*
		for( i = 0; i < g_SimulationSpec.nATC; ++ i ){//ATC의 HP에 놓여있는 적하 작업의 ID를 모은다.
			m_pEquipmentManager->GetProcessIDListOfATC( i, jobIDList );
		}
		*/
		if( jobIDList.empty() == false ){
			deque<string>::iterator index;
			deque<CALVJob*> jobSequence;
			for( index = jobIDList.begin(); index != jobIDList.end(); ++index ){
				//한 번 뿌려준 작업을 다시 뿌려주는 것을 방지하기 위하여
				vector<string>::iterator where;
				where = find( m_UnscheduledALVJobList.begin(), m_UnscheduledALVJobList.end(), *index );
				if( where != m_UnscheduledALVJobList.end() ){//뿌려준 적이 없는 작업이라면
					m_UnscheduledALVJobList.erase(where);
					jobSequence.push_back(m_ProcessMap[GETPROCESSID(*index)]->ALVJob);
				}				
			}
			if( jobSequence.empty() == false ){
				m_pEquipmentManager->AddNewALVJobSequence( jobSequence );
			}
		}
		
	}
	
	return;
}
//public operations----------------------------------------------------------------------------
void CProcessManager::Initialize( CALVManager* pEquipmentManager )
{
	//mapping pointer of Apron
	m_pEquipmentManager = pEquipmentManager;

	//initializing processes
	srand(g_SimulationSpec.randSEED);

	MakeSequenceOfProcess();

	//check the creation of processes
	int n40Process = 250 * (g_SimulationSpec.nShipBayPerQC - g_SimulationSpec.n20x20BayPerQC) * g_SimulationSpec.nQC;
	int n20x20Process = 2 * 250 * g_SimulationSpec.n20x20BayPerQC * g_SimulationSpec.nQC;
	m_nTotalProcess		= n40Process + n20x20Process;
	int nSizeProcessMap	= (int)m_ProcessMap.size();
	//////////////////////////////////////////////////////////////////////////
	m_nTotalProcess		= nSizeProcessMap;

	//프로세스를 QC와 Block에 배분
	DistributeProcesses();

	srand( (unsigned)time(NULL) );
	return;
}

void CProcessManager::ProcessFinished(CProcess* pProcess)
{
	//...check the end condition of simulation
	++m_nFinishedProcess;
	if( m_nTotalProcess <= m_nFinishedProcess ){
		m_pEquipmentManager->InformEmulationFinished();
	}
	return;	
}

CProcess* CProcessManager::GetJob(int AHVID, int& priority, int designatedQCID)
{
	if( m_ProcessMap.empty() ){
		return NULL;
	}
	
	CProcess* pProcess;
	map<int, CProcess*>::iterator prsIndex = m_ProcessMap.begin();

	//select a process
	pProcess = NULL;

	if( designatedQCID != -1 ){
		while(prsIndex != m_ProcessMap.end()  ){
			if( designatedQCID == prsIndex->second->iQCID){
				pProcess = prsIndex->second;
				break;
			}
			else{
				++prsIndex;
			}			
		}
	}
	if( pProcess == NULL ){
		pProcess = prsIndex->second;
	}

	priority = ++m_processPriority;
	m_ProcessMap.erase( prsIndex );
	pProcess->ALVJob->assignedEquipID = AHVID;

	return pProcess;
}

long CProcessManager::EstimateDeadline( int nSet, vector<string>& setHP, vector<string>& setALVJob, vector<string>& qcSchedule )
{
	CProcess* pProcess;
	vector<string>::iterator index;
	vector<string>::iterator indexHP;
	vector<string>::iterator indexALVJob;

	int currentTEU = 0;
	//const int maximumTEU = 8;
	const int maximumTEU = 4;
	string prsID;
	int	iProcessID;

	//현재 HP의 TEU를 측정한다. 참고. Maximum TEU of QC'HP = 8	
	//지금은 양하용 HP와 적하용 HP를 나눠 쓰므로 HP 여유 공간 계산은 양하만 ~
	for( indexHP = setHP.begin(); indexHP != setHP.end(); ++indexHP ){
		iProcessID = GETPROCESSID(*indexHP);
		pProcess = m_ProcessMap[iProcessID];
		if( pProcess->prsType == PrsType_Discharging ){
			if( pProcess->ALVJob->container.conType == ConType_General20ft ){
				currentTEU += 1;
			}
			else if( pProcess->ALVJob->container.conType == ConType_General40ft ){
				currentTEU += 2;
			}
		}
	}
	
	long elapsedTime = 0;
	index = qcSchedule.begin();

	int i;
	while( index != qcSchedule.end() || qcSchedule.empty() == false ){//QC의 cycle_time 단위로 HP 상태 갱신

		//남은 Schedule의 첫번째 작업을 가져온다.
		index = qcSchedule.begin();
		iProcessID = GETPROCESSID(*index);
		pProcess = m_ProcessMap[iProcessID];

		if( pProcess->prsType == PrsType_Loading ){//적하 작업인 경우
			bool loadingPossible = true;//적하 작업 수행 가능 여부
			for( i=0; i<nSet; ++i){				
				indexHP = find(setHP.begin(), setHP.end(), pProcess->prsID);//적하 컨테이너가 HP에 있는지 확인
				if( indexHP == setHP.end() ){//적하 컨테이너 Set 중에 하나라도 도착하지 않았다면 --> 적하 작업 수행 불가
					loadingPossible = false;
					break;
				}
				else{//작업 목록 갱신
					/*//현재 양하용 HP 적하용 HP 나눠 쓰므로 HP 여유 공간 계산은 양하만 ~
					if( pProcess->ALVJob->container.conType == ConType_General20ft ){//HP 남은 공간 갱신
						currentTEU -= 1;
					}
					else{
						currentTEU -= 2;
					}
					*/
					setHP.erase(indexHP);	//HP상황 해제
					qcSchedule.erase(index);
					if( qcSchedule.empty() ){
						break;
					}
					index = qcSchedule.begin();
					iProcessID = GETPROCESSID(*index);
					pProcess = m_ProcessMap[iProcessID];
				}
			}
			if( loadingPossible == false  ){//적하 작업 수행 불가 - Large Number Return
				//return g_SimulationSpec.halfCycletime*3;
				return 86400;
			}
		}
		else if( pProcess->prsType == PrsType_Discharging ){
			bool dischargingPossible = true;//양하 작업 수행 가능 여부
			for( i=0; i<nSet; ++i){
				if( pProcess->ALVJob->container.conType == ConType_General20ft ){//20x20ft 작업인 경우
					currentTEU += 1;
				}
				else{
					currentTEU += 2;
				}

				if( currentTEU > maximumTEU){//더 이상 HP에 놓을 수 없는 경우
					dischargingPossible = false;
					break;
				}
				else{//아직 여유공간이 있는 경우
					setHP.push_back(pProcess->prsID);	//HP상황 갱신
					qcSchedule.erase(index);
					if( qcSchedule.empty() ){
						break;
					}
					index = qcSchedule.begin();
					iProcessID = GETPROCESSID(*index);
					pProcess = m_ProcessMap[iProcessID];
				}						
			}
			if( dischargingPossible == false ){
				return (elapsedTime + g_SimulationSpec.halfCycletime);//지금 수행 불가능한 양하 작업이 도착하는 시점을 더해줌
			}
		}

		//ALV의 작업 완료 시점을 예측하여 HP 상태 갱신
		vector<string>::iterator releaseIndex;
		for( indexALVJob = setALVJob.begin(); indexALVJob != setALVJob.end(); ++indexALVJob ){
			iProcessID = GETPROCESSID(*indexALVJob);
			pProcess = m_ProcessMap[iProcessID];
			int traveltime = m_pEquipmentManager->GetTravelTimeEstimated(pProcess->ALVJob->assignedEquipID, pProcess->ALVJob );
			if( elapsedTime <= traveltime && traveltime <= (elapsedTime+g_SimulationSpec.halfCycletime) ){//작업 수행 완료 가능하다고 본다.
				if( pProcess->prsType == PrsType_Discharging ){				
					indexHP = find(setHP.begin(), setHP.end(), *indexALVJob);
					if( indexHP != setHP.end() ){
						setHP.erase(indexHP);			//HP에 컨테이너가 있다면 삭제
						if( (int)setALVJob.size() == 1){
							setALVJob.erase(indexALVJob);
							break;
						}
						else{
							indexALVJob = setALVJob.erase(indexALVJob);
							if( indexALVJob == setALVJob.end() ){
								break;
							}
						}
						if( pProcess->ALVJob->container.conType == ConType_General20ft ){//HP 상황 갱신
							currentTEU += 1;
						}
						else{
							currentTEU += 2;
						}
					}
				}
				else if( pProcess->prsType == PrsType_Loading ){
					setHP.push_back(pProcess->prsID);
					if( (int)setALVJob.size() == 1){
						setALVJob.erase(indexALVJob);
						break;
					}
					else{
						indexALVJob = setALVJob.erase(indexALVJob);
						if( indexALVJob == setALVJob.end() ){
							break;
						}
					}					
				}
			}
		}		
		
		elapsedTime += g_SimulationSpec.halfCycletime;
	}
	
	return elapsedTime;
}

string	CProcessManager::GetQuayHPID( string prsID )
{
	return m_ProcessMap[GETPROCESSID(prsID)]->ALVJob->QuayHPID;
}

CALVJob* CProcessManager::GetALVJob(string jobID)
{
	return m_ProcessMap[GETPROCESSID(jobID)]->ALVJob;
}