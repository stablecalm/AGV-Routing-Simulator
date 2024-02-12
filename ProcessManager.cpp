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

	//������ CProcess�� CALVJob�� ��� �����Ѵ�.
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

	//ALVJob ���� ����
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
	//�� ũ���� �� �� ���� ���� 2���� ����
	//�� ���̴� 25���� stack���� �Ǿ�������, �� stack�� 10 row�� �����Ǿ�����
	//single spreader type�� crane�� �� ���� �� ���̸� ó�� 
	//twin spreader type�� crane�� �� ���� ���� �� ���� ó��
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
	int prsID = 0;//���μ��� ID
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
						if( bType == BayType_20ft ){//20ft x 20ft ���� ���� - ���� �� ���� ����� ��ġ�� �����ϴ�.						
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
						if( bType == BayType_20ft ){//20ft x 20ft ���� ����
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
	//The simple and best way -��-
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
				m_QCProcessSequence[qc].push_back(process);		//ALV ������ ���μ��� ��� ����
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
		//���� QC�� HP�� �����ִ� ���� �۾��� ID�� HP�� ���� ù��° ���� �۾��� ID�� ������..
		for( i = 0; i < g_SimulationSpec.nQC; ++i ){
			m_pEquipmentManager->GetProcessIDListOfQC( i, jobIDList );
		}
		/*
		for( i = 0; i < g_SimulationSpec.nATC; ++ i ){//ATC�� HP�� �����ִ� ���� �۾��� ID�� ������.
			m_pEquipmentManager->GetProcessIDListOfATC( i, jobIDList );
		}
		*/
		if( jobIDList.empty() == false ){
			deque<string>::iterator index;
			deque<CALVJob*> jobSequence;
			for( index = jobIDList.begin(); index != jobIDList.end(); ++index ){
				//�� �� �ѷ��� �۾��� �ٽ� �ѷ��ִ� ���� �����ϱ� ���Ͽ�
				vector<string>::iterator where;
				where = find( m_UnscheduledALVJobList.begin(), m_UnscheduledALVJobList.end(), *index );
				if( where != m_UnscheduledALVJobList.end() ){//�ѷ��� ���� ���� �۾��̶��
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

	//���μ����� QC�� Block�� ���
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

	//���� HP�� TEU�� �����Ѵ�. ����. Maximum TEU of QC'HP = 8	
	//������ ���Ͽ� HP�� ���Ͽ� HP�� ���� ���Ƿ� HP ���� ���� ����� ���ϸ� ~
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
	while( index != qcSchedule.end() || qcSchedule.empty() == false ){//QC�� cycle_time ������ HP ���� ����

		//���� Schedule�� ù��° �۾��� �����´�.
		index = qcSchedule.begin();
		iProcessID = GETPROCESSID(*index);
		pProcess = m_ProcessMap[iProcessID];

		if( pProcess->prsType == PrsType_Loading ){//���� �۾��� ���
			bool loadingPossible = true;//���� �۾� ���� ���� ����
			for( i=0; i<nSet; ++i){				
				indexHP = find(setHP.begin(), setHP.end(), pProcess->prsID);//���� �����̳ʰ� HP�� �ִ��� Ȯ��
				if( indexHP == setHP.end() ){//���� �����̳� Set �߿� �ϳ��� �������� �ʾҴٸ� --> ���� �۾� ���� �Ұ�
					loadingPossible = false;
					break;
				}
				else{//�۾� ��� ����
					/*//���� ���Ͽ� HP ���Ͽ� HP ���� ���Ƿ� HP ���� ���� ����� ���ϸ� ~
					if( pProcess->ALVJob->container.conType == ConType_General20ft ){//HP ���� ���� ����
						currentTEU -= 1;
					}
					else{
						currentTEU -= 2;
					}
					*/
					setHP.erase(indexHP);	//HP��Ȳ ����
					qcSchedule.erase(index);
					if( qcSchedule.empty() ){
						break;
					}
					index = qcSchedule.begin();
					iProcessID = GETPROCESSID(*index);
					pProcess = m_ProcessMap[iProcessID];
				}
			}
			if( loadingPossible == false  ){//���� �۾� ���� �Ұ� - Large Number Return
				//return g_SimulationSpec.halfCycletime*3;
				return 86400;
			}
		}
		else if( pProcess->prsType == PrsType_Discharging ){
			bool dischargingPossible = true;//���� �۾� ���� ���� ����
			for( i=0; i<nSet; ++i){
				if( pProcess->ALVJob->container.conType == ConType_General20ft ){//20x20ft �۾��� ���
					currentTEU += 1;
				}
				else{
					currentTEU += 2;
				}

				if( currentTEU > maximumTEU){//�� �̻� HP�� ���� �� ���� ���
					dischargingPossible = false;
					break;
				}
				else{//���� ���������� �ִ� ���
					setHP.push_back(pProcess->prsID);	//HP��Ȳ ����
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
				return (elapsedTime + g_SimulationSpec.halfCycletime);//���� ���� �Ұ����� ���� �۾��� �����ϴ� ������ ������
			}
		}

		//ALV�� �۾� �Ϸ� ������ �����Ͽ� HP ���� ����
		vector<string>::iterator releaseIndex;
		for( indexALVJob = setALVJob.begin(); indexALVJob != setALVJob.end(); ++indexALVJob ){
			iProcessID = GETPROCESSID(*indexALVJob);
			pProcess = m_ProcessMap[iProcessID];
			int traveltime = m_pEquipmentManager->GetTravelTimeEstimated(pProcess->ALVJob->assignedEquipID, pProcess->ALVJob );
			if( elapsedTime <= traveltime && traveltime <= (elapsedTime+g_SimulationSpec.halfCycletime) ){//�۾� ���� �Ϸ� �����ϴٰ� ����.
				if( pProcess->prsType == PrsType_Discharging ){				
					indexHP = find(setHP.begin(), setHP.end(), *indexALVJob);
					if( indexHP != setHP.end() ){
						setHP.erase(indexHP);			//HP�� �����̳ʰ� �ִٸ� ����
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
						if( pProcess->ALVJob->container.conType == ConType_General20ft ){//HP ��Ȳ ����
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