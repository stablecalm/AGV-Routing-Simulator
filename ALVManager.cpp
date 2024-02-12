#include "StdAfx.h"
#include ".\ALVManager.h"


//constructor, destructor---------------------------------------------------------------------
CALVManager::CALVManager(void)
:m_ClockTick(0), m_ID("YT00"), m_bInitComplete(false), m_bVesselJobFinished(false), m_bEmulationFinised(false)
{
	
}

CALVManager::~CALVManager(void)
{
	PauseSimulation();

	//ALVInfo �޸� ����
	map<string, CALVInfo*>::iterator indexALVInfo;
	for( indexALVInfo = m_ALVInfo.begin(); indexALVInfo != m_ALVInfo.end(); ++indexALVInfo ){
		CALVInfo*	releaseALVInfo = (*indexALVInfo).second;
		if( releaseALVInfo != NULL )
			delete releaseALVInfo;
	}
	m_ALVInfo.clear();

	m_UnassignedJob.clear();
	m_AssignedJob.clear();
	
	m_WaitQCPermission.clear();			//HP �Ҵ��� ��ٸ��� ALV ID�� queue
	m_WaitBlockPermission.clear();	//HP ���࿡ ������ ALV ID�� queue

	m_ATCs.clear();
	m_QCs.clear();

	if( m_pALVScheduler != NULL ){
		delete m_pALVScheduler;
	}
	
}

//drawing operation--------------------------------------------------------------------------------------------
void CALVManager::DrawHPs( CDC& dc, double dZoom )
{
	int atc;
	for( atc = 0; atc < g_SimulationSpec.nATC; ++atc ){
		m_ATCs[atc].DrawHPs( dc, dZoom );
	}

	//QC �ùķ��̼�
	int qc;
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		m_QCs[qc].DrawHPs( dc, dZoom );
	}
}

void CALVManager::DrawCranes( CDC& dc, double dZoom )
{
	int atc;
	for( atc = 0; atc < g_SimulationSpec.nATC; ++atc ){
		m_ATCs[atc].DrawCrane( dc, dZoom );
	}

	//QC �ùķ��̼�
	int qc;
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		m_QCs[qc].DrawCrane( dc, dZoom );
	}
}

//public operation----------------------------------------------------------------------------
void CALVManager::StartSimulation(){
	m_ThreadTerminate = false;
	m_hThreadSimulation = (HANDLE)_beginthreadex( NULL, 0, &CALVManager::ThreadProcSimulation, (void*)this, 0, &m_ThreadSimulationID );
	return;
}

void CALVManager::PauseSimulation(){
	if( m_hThreadSimulation != 0 ){
		TerminateThread( m_hThreadSimulation, 0 );
		m_hThreadSimulation = 0;
	}
	return;
}

void CALVManager::FinishSimulation(){
	if( m_hThreadSimulation != 0 ){
		m_ThreadTerminate = true;
		WaitForSingleObject(m_hThreadSimulation, INFINITE);
		CloseHandle(m_hThreadSimulation);	
	}	
	return;
}

unsigned int CALVManager::ThreadProcSimulation(void* pArguments)
{
	CALVManager* pEM = (CALVManager*)pArguments;
	while( pEM->m_ThreadTerminate == false )
	{
		pEM->DoSingleIteration();
		if( g_SimulationSpec.delay > 0 ){
			Sleep( g_SimulationSpec.delay );
		}
		if( pEM->m_bEmulationFinised ){
			pEM->WriteResult();
			pEM->m_ThreadTerminate = true;
		}
	}
	_endthreadex(0);
	return 0;
}

void CALVManager::NonThreadSimulation(){
	//Thread�� ������� �������μ� ȭ�� ����� ���� �ʰ� �ùķ��̼�
	while(true){
		DoSingleIteration();
	}
	return;
}

void CALVManager::InitializeManager( CProcessManager* pProcessManager, CALVEmulator* pALVEmulator )
{
	m_pProcessManager = pProcessManager;
	m_pALVEmulator = pALVEmulator;
	m_pALVScheduler = new CALVScheduler(this);

	return;	
}

void CALVManager::InitializeAgentInfo()
{
	//QC, ATC, HP ���� ��, Apron�� HP ��ġ �˸�
	GenerateCranes();
	//Apron �ʱ�ȭ
	m_pALVEmulator->InitializeTrafficEnvironment();

	m_bInitComplete = true;
	return;
}

void CALVManager::DoSingleIteration()
{
	//cranes simulation
	DoSingleIterationforCranes();

	//traffic control
	HandleRequestDispatching();		//���μ����Ҵ� ��û�� ó���Ѵ�.(��, � QC�� ���� ���ΰ��� �����Ѵ�.)
	HandleRequestQCPermission();	//QC���Կ�û�� ó���Ѵ�. (��, QC�� � TP�� Container�� �ִ��� Ȯ���Ѵ�.)
	HandleRequestBlockPermission();

	//alv simulation
	m_pALVEmulator->DoSingleIteration();

	++m_ClockTick;	
	return;
}

void CALVManager::InitializeALVInformation(string ID, CPoint ptVehicle)
{
	CALVInfo* ALVInfo = new CALVInfo(ID, JobState_Idle, ptVehicle );
	m_ALVInfo.insert( make_pair( ID, ALVInfo) );

	return;
}

void CALVManager::AddNewALVJobSequence( deque<CALVJob*> jobSequence )
{
	//�̹� �ִ� ���� ALVJob*�� �� ���� �� �����Ƿ� ����
	map<string, CALVJob*>::iterator	unassignedJobIndex;
	map<string, CALVJob*>::iterator	assignedJobIndex;

	deque<CALVJob*>::iterator itrProcess;
	for( itrProcess = jobSequence.begin(); itrProcess != jobSequence.end(); ++itrProcess ){
		unassignedJobIndex = m_UnassignedJob.find((*itrProcess)->jobID);
		assignedJobIndex = m_AssignedJob.find((*itrProcess)->jobID);
		if( unassignedJobIndex == m_UnassignedJob.end() && assignedJobIndex == m_AssignedJob.end() ){//���ο� �۾��̸�
			m_UnassignedJob.insert( make_pair( (*itrProcess)->jobID, *itrProcess ) );
		}
		
	}
	return;
}

void CALVManager::UpdateALVInfo( string equipID, string jobID, CPoint pt, string wpID )
{
	//UPDATE ALV Information
	CALVJob* pCurrentJob = m_AssignedJob[jobID];

	EALVJobState jobState = pCurrentJob->jobState;
	m_ALVInfo[equipID]->UpdateInfo( jobState , pt );
	m_ALVInfo[equipID]->m_WPID = wpID;

	if( jobState == JobState_TransferAtQC ){
		if( pCurrentJob->prsType == PrsType_Discharging	){
			if( pCurrentJob->container.conType == ConType_General40ft){
				m_ALVInfo[equipID]->Load40ftContainer();
			}
			else if( pCurrentJob->container.conType == ConType_General20ft ){
				m_ALVInfo[equipID]->Load20ftContainer();
			}
		}
		else if( pCurrentJob->prsType == PrsType_Loading ){
			if( pCurrentJob->container.conType == ConType_General40ft){
				m_ALVInfo[equipID]->Unload40ftContainer();
			}
			else if( pCurrentJob->container.conType == ConType_General20ft ){
				m_ALVInfo[equipID]->Unload20ftContainer();
			}
		}
	}
	else if( jobState == JobState_TransferAtBlock ){
		if( pCurrentJob->prsType == PrsType_Discharging	){
			if( pCurrentJob->container.conType == ConType_General40ft){
				m_ALVInfo[equipID]->Unload40ftContainer();
			}
			else if( pCurrentJob->container.conType == ConType_General20ft ){
				m_ALVInfo[equipID]->Unload20ftContainer();
			}			
		}
		else if( pCurrentJob->prsType == PrsType_Loading ){
			if( pCurrentJob->container.conType == ConType_General40ft){
				m_ALVInfo[equipID]->Load40ftContainer();
			}
			else if( pCurrentJob->container.conType == ConType_General20ft ){
				m_ALVInfo[equipID]->Load20ftContainer();
			}
		}
	}

	return;
}

void CALVManager::UpdateALVJobState( string jobID, EALVJobState jobStateOperating )
{
	//UPDATE ALVJob Information
	CALVJob* pCurrentJob = m_AssignedJob[jobID];

	//���� ALV�� ���� ���� �۾� ���� ����
	pCurrentJob->SetJobState(jobStateOperating, (long)(m_ClockTick * g_SimulationSpec.unitTime));

	if( jobStateOperating == JobState_Finished ){
		DeclareJobFinished( pCurrentJob->assignedEquipID, jobID);
	}
	return;
}

void CALVManager::UpdateALVJobInfo( string jobID, long elapsedTime, string hpID )
{
	//UPDATE ALVJob Information
	CALVJob* pCurrentJob = m_AssignedJob[jobID];

	EALVJobState completeJobState = pCurrentJob->jobState;

	pCurrentJob->SetEndTimeOf( completeJobState, elapsedTime );

	//�۾� ���� ����� ���� HP ���� ����
	if( completeJobState == JobState_TransferAtQC ){
		if( pCurrentJob->prsType == PrsType_Discharging ){
			int iCraneID = GETCRANEID(pCurrentJob->QCID);
			m_QCs[iCraneID].ReleaseHPContainer( pCurrentJob->GetQuayHP(), pCurrentJob->container );
		}
		else if( pCurrentJob->prsType == PrsType_Loading ){
			int iCraneID = GETCRANEID(pCurrentJob->QCID);
			m_QCs[iCraneID].SetHPContainer( pCurrentJob->GetQuayHP(), pCurrentJob->container );
		}		
	}
	else if( completeJobState ==  JobState_TransferAtBlock ){
		if( pCurrentJob->prsType == PrsType_Discharging ){
			int iCraneID = GETCRANEID(pCurrentJob->BlockID);
			m_ATCs[iCraneID].SetHPContainer( pCurrentJob->GetYardHP(), pCurrentJob->container );
		}
		else if( pCurrentJob->prsType == PrsType_Loading ){
			int iCraneID = GETCRANEID(pCurrentJob->BlockID);
			m_ATCs[iCraneID].ReleaseHPContainer( pCurrentJob->GetYardHP(), pCurrentJob->container );
		}
	}

	return;
}

void CALVManager::DeclareJobFinished( string equipID, string jobID )
{
	//ALV ���� ����
	m_ALVInfo[equipID]->DeleteJob(jobID);

	//Scheduler�� �˸�
	m_pALVScheduler->InformJobComplete( m_AssignedJob[jobID] );

	//�Ϸ�� Job ����
	map<string, CALVJob*>::iterator index;
	for( index = m_AssignedJob.begin(); index != m_AssignedJob.end(); ++index){
		if( index->first == jobID ){
			m_ALVJobStatistics.AddCompleteALVJob( index->second );
			m_AssignedJob.erase( index );
			break;
		}
	}	
	return;	
}

//private operation---------------------------------------------------------------------------
void CALVManager::HandleRequestDispatching()
{
	//���⿡ ��ȹ ���� ���� �ڵ带 �߰��մϴ�.
	if( m_ClockTick%2000 == 0 ){//10�ʸ��� �� ���� HP�� ũ���� �۾� ��ȹ �ݿ�
		m_pProcessManager->DistributeALVJob();
		if( m_UnassignedJob.empty() == false ){//�ʿ��� ������ �۾� ��ȹ
			m_pALVScheduler->MakeSchedule(m_ALVInfo, m_AssignedJob, m_UnassignedJob );
		}
	}
	/*
	if( m_UnassignedJob.empty() == false ){//�ʿ��� ������ �۾� ��ȹ
		m_pALVScheduler->MakeSchedule(m_ALVInfo, m_AssignedJob, m_UnassignedJob );
	}
	*/
	return;
}

void CALVManager::HandleRequestQCPermission()
{
	if( m_WaitQCPermission.empty() )
		return;

	vector<HPRsvInfo>::iterator index;//ALVID, (CRANEID, JOBID)

	//QC ���� �� HP ��� ����
	for( index = m_WaitQCPermission.begin(); index != m_WaitQCPermission.end(); ){
		CALVJob* pALVJob = m_AssignedJob[index->jobID];
		CALVJob* pConnectedJob = NULL;
		if( pALVJob->connectedJobList.empty() == false ){
			pConnectedJob = m_pProcessManager->GetALVJob(pALVJob->connectedJobList[0].second);
		}

		if( pALVJob->qcHPReservationComplete == false ){
			//���� �۾� ���¸� ���� HP�� ��� ����(���� ����)�� ������.
			bool bFixedReservation = false;	//������ HP�� �����ϴ� ������� ?
			bool bPassPoint = false;	//HP�� �̵� ����� ��������� ����ϱ� ���� ������� ?		
			if( pALVJob->jobState == JobState_TravelToDesignatedQC ){//Job�� ������ QC�� �̵��ϴ� ���
				if( pALVJob->prsType == PrsType_Discharging ){
					//���� �� ������ QC�� �̵��ϴ� ���, ������ HP�� �����ϴ� ����̴�.
					bFixedReservation = true;
				}
				else if( pALVJob->prsType == PrsType_Loading ){
					bFixedReservation = false;
					if( pConnectedJob != NULL ){
						if( pConnectedJob->QuayHPID != "" ){
							bFixedReservation = true;
						}
					}					
				}
			}
			else{
				bPassPoint = true;
			}

			//HP ���� ������ ���� ���� �õ�
			int qcID = m_QCIDList[index->craneID];
			if( bFixedReservation ){//������ HP�� �����ϴ� ���
				if( pALVJob->QuayHPID != "" ){
					if( m_QCs[qcID].GetCurrentCapacityofHP(pALVJob->QuayHPID) ){
						if( m_QCs[qcID].SetHPReservation(pALVJob->QuayHPID) ){
							//������  ������ ��� - ALVJob�� ���� ���¸� �����Ѵ�.
							pALVJob->qcHPReservationComplete = true;
						}
					}		
				}				
			}
			else{
				int destinationHPID = -1;
				if( bPassPoint ){//���� �� ���İ��� ���� ������ ���
					destinationHPID = m_QCs[qcID].GetReservableHPforPassPoint();		//���� ������ HP�� �޴´�.
				}
				else{//�����̳ʸ� ���������� ���� ���
					//���� �۾�
					//A. QC�� �޾Ƶ��� �� �ִ� �۾����� Ȯ��
					if( m_QCs[qcID].CheckAdmissibleLoad(index->jobID)){
						if( m_AssignedJob[index->jobID]->connectedJobList.empty() == false ){
							string connectedJob = m_AssignedJob[index->jobID]->connectedJobList[0].second;
							string hpStringID = m_pProcessManager->GetQuayHPID( connectedJob );
							if( hpStringID != "" ){
								if( m_QCs[qcID].CheckReservable(hpStringID, index->needCapacity) ){//������ HP�� ���� �������� Ȯ���Ѵ�.
									destinationHPID = m_QCs[qcID].GetHPIntID(hpStringID);
								}
							}
							else{
								destinationHPID = m_QCs[qcID].GetReservableHPfor(index->needCapacity);	//���డ���ϰ� ����ִ� HP�� �޴´�.
							}
						}
						else{
							destinationHPID = m_QCs[qcID].GetReservableHPfor(index->needCapacity);	//���డ���ϰ� ����ִ� HP�� �޴´�.
						}
					}
				}
				if( destinationHPID != -1 ){//��밡���� HP�� �Ҵ� ���� ���
					if( m_QCs[qcID].SetHPReservation(destinationHPID) ){//�ش� HP�� �����ϰ� �۾� ������ ������Ʈ�Ѵ�.
						string destinationHPStringID = m_QCs[qcID].GetHPStringID( destinationHPID );
						pALVJob->QuayHPID = destinationHPStringID;
						if( pConnectedJob != NULL ){//����� �۾��� HP�� �����Ѵ�.(QC����)
							pConnectedJob->QuayHPID = destinationHPStringID;
						}
						pALVJob->qcHPReservationComplete = true;	//���� ���¸� �����Ѵ�.
					}
				}
			}

			if( pALVJob->qcHPReservationComplete == false ){
				++index;
				continue;
			}
			else{
				//������ ������ ���, �۾� �ð� ���� ����
				pALVJob->SetEndTimeOf(JobState_WaitQCPermission, (long)(m_ClockTick * g_SimulationSpec.unitTime));

				//�Ϸ�� �� ����
				if( m_WaitQCPermission.size() == 1 ){
					m_WaitQCPermission.erase( index );
					break;
				}
				else if( index == m_WaitQCPermission.begin() ){
					m_WaitQCPermission.erase( index );
					index = m_WaitQCPermission.begin();
				}
				else{
					vector<HPRsvInfo>::iterator index2;
					index2 = index;
					--index;
					m_WaitQCPermission.erase( index2 );
					++index;
				}
			}		
		}
		else{//�̹� �Ϸ�� �� ����
			if( m_WaitQCPermission.size() == 1 ){
				m_WaitQCPermission.erase( index );
				break;
			}
			else if( index == m_WaitQCPermission.begin() ){
				m_WaitQCPermission.erase( index );
				index = m_WaitQCPermission.begin();
			}
			else{
				vector<HPRsvInfo>::iterator index2;
				index2 = index;
				--index;
				m_WaitQCPermission.erase( index2 );
				++index;
			}
		}
		
		
	}

	return;
}

void CALVManager::HandleRequestBlockPermission()		//ALV�� ���� �� Block�� �۾���� TP�� �ľ��ϱ� ���� �Լ�. (�� TP�� �޸��� ��쿡�� ������� ����)
{
	if( m_WaitBlockPermission.empty() )
		return;

	vector<HPRsvInfo>::iterator index;

	//QC ���� �� HP ��� ����
	for( index = m_WaitBlockPermission.begin(); index != m_WaitBlockPermission.end(); ){
		CALVJob* pALVJob = m_AssignedJob[index->jobID];

		if( pALVJob->blockHPReservationComplete == false ){
			int atcID = m_ATCIDList[index->craneID];	//�۾� ��� ���
			int hpID = -1;
						
			//HP ����
			if( pALVJob->prsType == PrsType_Loading ){//���� ��, ������ HP�� �����ϴ� ����̴�.
				if( pALVJob->YardHPID != "" ){
					hpID = m_ATCs[atcID].GetHPIntID(pALVJob->YardHPID);					
				}
			}
			else{
				hpID = m_ATCs[atcID].GetReservableHPfor(index->needCapacity);
			}

			//HP ����
			if( hpID != -1 ){
				if( m_ATCs[atcID].CheckPairHPReservation(hpID) ){
					if( m_ATCs[atcID].SetHPReservation(hpID) ){
						string destinationHPStringID = m_ATCs[atcID].GetHPStringID( hpID );
						pALVJob->YardHPID = destinationHPStringID;
						pALVJob->blockHPReservationComplete = true;//���� ���¸� �����Ѵ�.
					}						
				}					
			}			

			if( pALVJob->blockHPReservationComplete == false ){
				++index;
				continue;
			}
			else{
				//������ ������ ���, �۾� �ð� ���� ����
				pALVJob->SetEndTimeOf(JobState_WaitBlockPermission, (long)(m_ClockTick * g_SimulationSpec.unitTime));

				//�Ϸ�� �� ����
				if( m_WaitBlockPermission.size() == 1 ){
					m_WaitBlockPermission.erase( index );
					break;
				}
				else if( index == m_WaitBlockPermission.begin() ){
					m_WaitBlockPermission.erase( index );
					index = m_WaitBlockPermission.begin();
				}
				else{
					vector<HPRsvInfo>::iterator index2;
					index2 = index;
					--index;
					m_WaitBlockPermission.erase( index2 );
					++index;
				}
			}		
		}
		else{
			//�̹� �Ϸ�� ���� �ߺ����� ���� ���
			if( m_WaitBlockPermission.size() == 1 ){
				m_WaitBlockPermission.erase( index );
				break;
			}
			else if( index == m_WaitBlockPermission.begin() ){
				m_WaitBlockPermission.erase( index );
				index = m_WaitBlockPermission.begin();
			}
			else{
				vector<HPRsvInfo>::iterator index2;
				index2 = index;
				--index;
				m_WaitBlockPermission.erase( index2 );
				++index;
			}
		}
	}

	return;
}

void CALVManager::RequestHPReservation( HPType hpType, HPRsvInfo rsvInfo )
{
	if( hpType == HPType_QuayHP ){
		m_WaitQCPermission.push_back( rsvInfo );
	}
	else if( hpType == HPType_SeasideYardHP ){
		m_WaitBlockPermission.push_back( rsvInfo );
	}

	return;
}

void CALVManager::ConfirmHPReservation( HPType hpType, HPRsvInfo rsvInfo )
{
	if( hpType == HPType_QuayHP ){
		if( m_WaitQCPermission.empty() == false ){
			vector<HPRsvInfo>::iterator index;
			for( index = m_WaitQCPermission.begin(); index != m_WaitQCPermission.end(); ++index ){
				if( rsvInfo == (*index) )
					return;
			}
			m_WaitQCPermission.push_back(rsvInfo);
		}
		else{
			m_WaitQCPermission.push_back(rsvInfo);
		}		
	}
	else if( hpType == HPType_SeasideYardHP ){
		if( m_WaitBlockPermission.empty() == false ){
			vector<HPRsvInfo>::iterator index;
			for( index = m_WaitBlockPermission.begin(); index != m_WaitBlockPermission.end(); ++index ){
				if( rsvInfo == (*index) )
					return;
			}
			m_WaitBlockPermission.push_back(rsvInfo);
		}
		else{
			m_WaitBlockPermission.push_back(rsvInfo);
		}
		
	}
	return;
}

void CALVManager::ReleaseHP( HPType hpType, string hpID )
{
	//m_pALVJobManager->RequestHPRelease( hpID );	

	ReleaseHPNonDiAlogVersion( hpType, hpID );

	return;
}

void CALVManager::InformVesselJobFinished()
{
	m_bVesselJobFinished = true;
	m_pALVEmulator->InformVesselJobFinished();
	return;
}

int CALVManager::GetDistanceBetween( string equipID, HPType hpType, string craneID )
{
	return m_pALVEmulator->GetDistanceBetween( equipID, hpType, craneID);	
}

void CALVManager::WriteResult()
{
	ofstream ofs;
	CString resultFileName;

	//QC Result ���
	resultFileName.Format("RESULT_QC_PRODUCTIVITY.txt");
	ofs.open(resultFileName, ios_base::app);
	if(ofs.fail()){
		CString msg;
		msg.Format(resultFileName+" ��� ���� �б� ����");
		AfxMessageBox(_T(msg),MB_OK,0);
		return;
	}
	
	//��� ���� 
	ofs.seekp(ios_base::end);
	
	//PrsType(L/D)
	if( g_SimulationSpec.cargoType == CargoType_Discharge ){	
		ofs << "Discharge";
	}
	else if( g_SimulationSpec.cargoType == CargoType_Load ){
		ofs << "Load"; 
	}
	else if( g_SimulationSpec.cargoType == CargoType_Mixed ){
		ofs << "Mixed"; 
	}
	else if( g_SimulationSpec.cargoType == CargoType_DoubleCycle ){
		ofs << "Double"; 
	}
	ofs << " ";
	//Dispatch(I/E/L/N)
	switch(g_SimulationSpec.dispType){
		case DR_MinimumInventory:
			ofs << "I";
			break;
		case DR_EarliestDeadline:
			ofs << "E";
			break;
		case DR_HurdleJump:
			ofs << "H";
			break;
		case DR_GA:
			ofs << "G";
			break;
		default:
			ofs << "X";
			break;
	}
	ofs << " ";
	//nAGV
	ofs	<< g_SimulationSpec.nVehicle << " ";
	ofs << m_ClockTick << " ";//�۾��������
	ofs << "Avg ";
	
	//AvgThroughputofQC AvgIdleTimeofQC
	int throughput = 0;
	int idleTime = 0;
	int qc;
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc )
	{
		throughput += m_QCs[qc].GetThroughputPerHour();
		idleTime += (int)(m_QCs[qc].GetIdleTime()*g_SimulationSpec.unitTime);
	}
	ofs << throughput/g_SimulationSpec.nQC << " "
		<< (double)(idleTime/g_SimulationSpec.nQC) << " ";
	
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc )
	{
		if( m_QCs[qc].GetSpreaderType() == Spreader_Single ){
			ofs << "Single ";
		}
		else if( m_QCs[qc].GetSpreaderType() == Spreader_Twin ){
			ofs << "Twin ";
		}
		ofs << m_QCs[qc].GetThroughputPerHour() << " ";
		ofs << (int)(m_QCs[qc].GetIdleTime()*g_SimulationSpec.unitTime) << " ";
	}

	//QC�� �������Ŭ ���� Ƚ��
	ofs << " DoubleCylce ";
	int avg = 0;
	for( int qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		avg += m_QCs[qc].GetnDoubleCycle();
	}
	avg /= g_SimulationSpec.nQC;
	ofs << avg << " ";
	for( int qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		ofs << m_QCs[qc].GetnDoubleCycle() << " ";
	}
	ofs	<< endl;
	ofs.close();


	//ALV ������� Result ���
	//m_pALVEmulator->WriteResult();
	
	//ALVJob ������� Result ���
	m_ALVJobStatistics.WriteResult();

	return;
}


//////////////////////////////////////////////////////////////////////////
//for ver.Lofty_Hound Only
//////////////////////////////////////////////////////////////////////////
void CALVManager::GenerateCranes()
{
	ostringstream sout;

	//ATC�ʱ�ȭ
	m_ATCs.clear();
	m_ATCIDList.clear();

	int atc;
	CPoint atcPos;
	atcPos.x = g_SimulationSpec.GetXposYard();
	atcPos.y = g_SimulationSpec.GetYposYard();

	for( atc = 0; atc < g_SimulationSpec.nATC; ++atc ){
		m_ATCs.push_back( CATC(atc, atcPos) );
		m_ATCIDList.insert( make_pair( m_ATCs[atc].GetStringID(), atc) );

		//ATC�� HP ����
		int hpID;
		CPoint HPpos;
		//������κ��� ù��° ���� TP �߰�
		HPpos = atcPos;
		HPpos.x += g_SimulationSpec.GetXposBlockHP();
		HPpos.y -= g_SimulationSpec.Container40ftHalfLength;

		for( hpID = 0; hpID < g_SimulationSpec.nHPperBlock; ++hpID ){
			CHP hp = CHP( hpID, HPpos, HPA_Vertical );
			sout << m_ATCs[atc].GetStringID() << "S" << hpID;
			hp.SetStringID( sout.str() );
			sout.str("");

			//Apron�� Waypoint�� �����Ѵ�.
			m_pALVEmulator->SetWaypoint( hp.GetStringID(), HPType_SeasideYardHP, HPpos );

			m_ATCs[atc].SetHP( hp );
			HPpos.x += g_SimulationSpec.blockHPGap;
		}

		if( g_SimulationSpec.nRowperBlock == 2 ){		//������κ��� �ι�° ���� TP �߰�
			HPpos = atcPos;
			HPpos.x += g_SimulationSpec.GetXposBlockHP();
			HPpos.y -= (g_SimulationSpec.Container40ftHalfLength + g_SimulationSpec.Container40ftLength);
			for( hpID = 0; hpID < g_SimulationSpec.nHPperBlock; ++hpID ){
				CHP hp = CHP( hpID + g_SimulationSpec.nHPperBlock, HPpos, HPA_Vertical );
				sout << m_ATCs[atc].GetStringID() << "S" << hpID + g_SimulationSpec.nHPperBlock;
				hp.SetStringID( sout.str() );
				sout.str("");

				//Apron�� Waypoint�� �����Ѵ�.
				m_pALVEmulator->SetWaypoint( hp.GetStringID(), HPType_SeasideYardHP, HPpos );

				m_ATCs[atc].SetHP( hp );
				HPpos.x += g_SimulationSpec.blockHPGap;
			}
		}

		m_ATCs[atc].SetEquipmentManager( this );

		atcPos.x += g_SimulationSpec.blockWidth;
		atcPos.x += g_SimulationSpec.blockGap;
	}

	//QC�ʱ�ȭ
	int qc;
	m_QCs.clear();
	CPoint qcPos;
	qcPos.x = g_SimulationSpec.GetXposQC() - (int)(g_SimulationSpec.QClegWidth/2.0);
	qcPos.y = g_SimulationSpec.GetYposQC();

	//�����׸���ȸ ����� - ����(20x20), ����(20x20), ����(40)
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		if( qc == 0 ){
			m_QCs.push_back( CQC( Trolley_Single, Spreader_Twin, qc, qcPos) );
			m_QCs[qc].SetBayType( BayType_40ft );
			m_QCs[qc].SetCargoType( CargoType_Load );
		}
		else if( qc == 1){
			m_QCs.push_back( CQC( Trolley_Single, Spreader_Twin, qc, qcPos) );
			m_QCs[qc].SetBayType( BayType_40ft );
			m_QCs[qc].SetCargoType( CargoType_Load );
		}
		else if( qc == 2){
			m_QCs.push_back( CQC( Trolley_Single, Spreader_Twin, qc, qcPos) );
			m_QCs[qc].SetBayType( BayType_40ft );
			m_QCs[qc].SetCargoType( CargoType_Load );
		}

		m_QCIDList.insert( make_pair( m_QCs[qc].GetStringID(), qc) );

		//QC�� HP ����
		CPoint HPpos;
		HPpos.x = qcPos.x + (int)(g_SimulationSpec.QClegWidth/2);
		HPpos.y = g_SimulationSpec.boundaryOfQuaysideHP;

		int hpID;
		for( hpID = 0; hpID < g_SimulationSpec.nHPperQC; ++hpID ){
			CHP hp = CHP( hpID, HPpos, HPA_Horizontal);
			sout << m_QCs[qc].GetStringID() << "Q" << hpID;
			hp.SetStringID( sout.str() );
			sout.str("");

			//Apron�� Waypoint�� �����Ѵ�.
			m_pALVEmulator->SetWaypoint( hp.GetStringID(), HPType_QuayHP, HPpos );

			m_QCs[qc].SetHP( hp );
			HPpos.y += g_SimulationSpec.QCHPGap;
		}

		m_QCs[qc].SetEquipmentManager( this );

		qcPos.x += g_SimulationSpec.GetQCGap();
	}

	return;
}

void CALVManager::DoSingleIterationforCranes()
{
	//ATC �ùķ��̼�
	int atc;
	for( atc = 0; atc < g_SimulationSpec.nATC; ++atc ){
		m_ATCs[atc].DoSingleIteration();
	}

	//QC �ùķ��̼�
	int qc;
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		m_QCs[qc].DoSingleIteration();
	}

	return;
}

void CALVManager::ReleaseHPNonDiAlogVersion( HPType hpType, string hpID )
{
	if( hpType == HPType_QuayHP ){
		string qcID = hpID.substr(0,7);
		m_QCs[m_QCIDList[qcID]].ReleaseHPReservation( hpID );
	}
	else if( hpType == HPType_SeasideYardHP ){
		string atcID = hpID.substr(0,7);
		m_ATCs[m_ATCIDList[atcID]].ReleaseHPReservation( hpID );
	}
	return;
}

int	 CALVManager::GetIdleTimeOfQC( int qcID )
{ 
	return m_QCs[qcID].GetIdleTime(); 
}

void CALVManager::SetProcessSequence( EPrsType prsType, EEquipType equipType, int equipID, deque<CProcess*> &prsSequence )
{
	if( equipType == EquipType_QC ){
		if( prsType == PrsType_Discharging ){
			m_QCs[equipID].SetDischargingSequence( prsSequence );
		}
		else if( prsType == PrsType_Loading ){
			m_QCs[equipID].SetLoadingSequence( prsSequence );
		}		
	}
	else if( equipType == EquipType_ATC ){
		if( prsType == PrsType_Discharging ){
			//m_ATCs[equipID].SetDischargingSequence( prsSequence );
		}
		else if( prsType == PrsType_Loading ){
			//m_ATCs[equipID].SetLoadingSequence( prsSequence );
		}		
	}
	return;
}

void CALVManager::SetProcessSequence( EPrsType prsType, EEquipType equipType, int equipID, vector<CProcess*> &prsSequence )
{
	if( equipType == EquipType_QC ){
		if( prsType == PrsType_Discharging ){
			//m_QCs[equipID].SetDischargingSequence( prsSequence );
		}
		else if( prsType == PrsType_Loading ){
			//m_QCs[equipID].SetLoadingSequence( prsSequence );
		}		
	}
	else if( equipType == EquipType_ATC ){
		if( prsType == PrsType_Discharging ){
			m_ATCs[equipID].SetDischargingSequence( prsSequence );
		}
		else if( prsType == PrsType_Loading ){
			m_ATCs[equipID].SetLoadingSequence( prsSequence );
		}		
	}
	return;
}

void CALVManager::ProcessFinished(CProcess* prs)
{ 
	m_pProcessManager->ProcessFinished(prs); 
	return;
}

long CALVManager::EstimateDeadline( string qcID, int nSet, vector<string>& setHP, vector<string>& qcSchedule )
{
	vector<string> setALV;

	map<string, CALVJob*>::iterator index;
	for( index = m_AssignedJob.begin(); index != m_AssignedJob.end(); ++index ){
		if( index->second->QCID == qcID ){
			setALV.push_back(index->first);
		}
	}
	return m_pProcessManager->EstimateDeadline( nSet, setHP, setALV, qcSchedule);
}

bool CALVManager::CheckTopoPriority( string qcID, string jobID)
{
	return m_QCs[GETCRANEID(qcID)].CheckTopPriority(jobID);
}

void CALVManager::GenerateTravelTimeMap()
{
	pair<string, string> sourceDestination;
	int travelTime;

	ifstream ifs;
	//...simulation parameters
	ifs.open( "INFO_traveltime.txt" );
	while( ifs.fail() == false ){
		ifs >> sourceDestination.first;
		ifs >> sourceDestination.second;
		ifs >> travelTime;
		
		m_TravelTimeMap.insert( make_pair(sourceDestination, travelTime) );
		
	}
	ifs.close();

	return;
}

int CALVManager::GetTravelTimefromMap( string sourceID, string destinationID )
{
	if( sourceID == destinationID){
		return 0;
	}
	else if( sourceID.substr(0,2) == "YD" && destinationID.substr(0,2) == "YD"){//Block���� Block���� ���� ���
		if( sourceID.substr(0,8) == destinationID.substr(0,8) ){//���� ��� �� ��� �̵� �ð�
			return 23;
		}
		else{//��� �� ��� �̵� �ð� (�� ����� 37��)
			int sb = atoi(sourceID.substr(6,7).c_str());
			int db = atoi(destinationID.substr(6,7).c_str());
			return abs(sb-db)*37;
		}
	}
	else if( sourceID.substr(0,2) == "QC" && destinationID.substr(0,2) == "QC"){//QC-->QC
		int sq = atoi(sourceID.substr(6,7).c_str());
		int dq = atoi(destinationID.substr(6,7).c_str());
		if( dq == sq ){//STT�� ȣȯ�� ���ؼ�
			return 0;
		}
		else if( dq < sq){//�� ���� QC���� QC�� �ٷ� ���� ����̴�.
			return abs(sq-dq)*25;
		}
		else{//������ ���Ƽ� ���� �Ѵ�.
			return abs(dq-sq)*17+35+30*2;
		}
	}
	else if( sourceID.substr(0,2) == "WA" && destinationID.substr(0,2) == "WA"){
		int sq = atoi(sourceID.substr(2,4).c_str());
		int dq = atoi(destinationID.substr(2,4).c_str());
		return 26 + 2*abs(sq-dq);
	}
	else if( sourceID.substr(0,2) == "YD" && destinationID.substr(0,2) == "QC"){//Block <--> QC
		//(qc ID x 2)+1 �ϸ� ���� ����� ��ġ. ����� �Ÿ��� ���� ��� �ð��� ��ȯ
		int block = atoi(sourceID.substr(6,7).c_str());
		int qc = atoi(destinationID.substr(6,7).c_str());
		qc = qc*2+1;
		if( qc == 0 ) qc = 1;
		int relative = abs(block-qc);
		if( relative == 0 ) return 71;
		else if( relative == 1 ) return 81;
		else if( relative == 2 ) return 81;
		else if( relative == 3 ) return 89;
		else if( relative == 4 ) return 96;
		else if( relative == 5 ) return 103;
		else if( relative == 6 ) return 109;
		else  return 112;

	}
	else if( sourceID.substr(0,2) == "QC" && destinationID.substr(0,2) == "YD"){//Block <--> QC
		//(qc ID x 2)+1 �ϸ� ���� ����� ��ġ. ����� �Ÿ��� ���� ��� �ð��� ��ȯ
		int qc = atoi(sourceID.substr(6,7).c_str());
		int block = atoi(destinationID.substr(6,7).c_str());
		qc = qc*2+1;
		if( qc == 0 ) qc = 1;
		int relative = abs(block-qc);
		if( relative == 0 ) return 71;
		else if( relative == 1 ) return 81;
		else if( relative == 2 ) return 81;
		else if( relative == 3 ) return 89;
		else if( relative == 4 ) return 96;
		else if( relative == 5 ) return 103;
		else if( relative == 6 ) return 109;
		else  return 112;

	}
	else if( sourceID.substr(0,2) == "WA" && destinationID.substr(0,2) == "QC" ){//WA���� QC���� ���� ���
		return 30;//���� ������ ��ġ �� ���
	}
	else if( sourceID.substr(0,2) == "QC" && destinationID.substr(0,2) == "WA" ){//WA���� QC���� ���� ���
		return 30;//���� ������ ��ġ �� ���
	}
	else if( sourceID.substr(0,2) == "WA" && destinationID.substr(0,2) == "YD" ){//WA���� QC���� ���� ���
		int wa = atoi(sourceID.substr(2,4).c_str());
		int yd = atoi(destinationID.substr(6,7).c_str());
		wa /= 5;
		return 30 + abs(wa-yd)*10;
	}
	else if( sourceID.substr(0,2) == "YD" && destinationID.substr(0,2) == "WA" ){//WA���� QC���� ���� ���
		int yd = atoi(sourceID.substr(6,7).c_str());
		int wa = atoi(destinationID.substr(2,4).c_str());
		wa /= 5;
		return 30 + abs(wa-yd)*10;
	}
	
	return 100;//LeftsideGateway���
}

int CALVManager::GetTravelTimeEstimated( string equipID, CALVJob* job )
{
	//�� �Լ��� 
	
	int travelTime = 0;
	if( g_SimulationSpec.dispType == DR_DualSTT ){
		//'parameter�� �־��� ALV�� ���� ��ġ���� parameter�� �־��� �۾��� QC���� ���µ� �ɸ��� �ð�'
		//�� �����Ͽ� ��ȯ�Ѵ�.
		if( job->prsType == PrsType_Discharging ){
			travelTime += GetTravelTimefromMap( m_ALVInfo[equipID]->m_WPID, job->QCID );
			travelTime += g_SimulationSpec.avgWaitingTime; //HP ��� ��� �ð�
		}
		else{//����
			travelTime += GetTravelTimefromMap( m_ALVInfo[equipID]->m_WPID, job->BlockID);
			travelTime += GetTravelTimefromMap( job->BlockID, job->QCID  );
			travelTime += (41+32);//lifting�� lowering�� �ɸ��� �ð�
			travelTime += g_SimulationSpec.avgWaitingTime * 2; //HP ��� ��� �ð�
		}
	}
	else{
		//'parameter�� �־��� ALV�� ���� ��ġ���� parameter�� �־��� �۾��� �Ϸ��ϴµ� �ɸ��� �ð�'
		//�� �����Ͽ� ��ȯ�Ѵ�.
		if( job->prsType == PrsType_Discharging ){
			travelTime += GetTravelTimefromMap( m_ALVInfo[equipID]->m_WPID, job->QCID );
			travelTime += GetTravelTimefromMap( job->QCID, job->BlockID );
		}
		else{//����
			travelTime += GetTravelTimefromMap( m_ALVInfo[equipID]->m_WPID, job->BlockID);
			travelTime += GetTravelTimefromMap( job->BlockID, job->QCID  );
		}

		travelTime += (41+32);//lifting�� lowering�� �ɸ��� �ð�
		travelTime += g_SimulationSpec.avgWaitingTime * 2; //HP ��� ��� �ð�
	}
	
	return travelTime;
	
}

bool CALVManager::CheckQCSchedule(CProcess* pProcess)
{
	if( m_QCs[pProcess->iQCID].CheckAdmissibleLoad(pProcess->prsID) )
		return true;
	return false;
}

bool CALVManager::CheckQCSchedule(CALVJob* pALVJob)
{
	int qcID = GETCRANEID(pALVJob->QCID);
	if( m_QCs[qcID].CheckAdmissibleLoad(pALVJob->jobID) )
		return true;
	return false;
}

ETrolleyType CALVManager::GetQCTrolleyType( int qcID )
{
	return m_QCs[qcID].GetTrolleyType();
}

ESpreaderType CALVManager::GetQCSpreaderType( int qcID )
{
	return m_QCs[qcID].GetSpreaderType();
}

EBayType CALVManager::GetQCBayType( int qcID )
{
	return m_QCs[qcID].GetBayType();
}

ECargoType CALVManager::GetQCCargoType( int qcID )
{
	return m_QCs[qcID].GetCargoType();
}

void CALVManager::SetCraneSchedule( int qc, deque<CProcess*> craneSchedule)
{
	//m_pALVScheduler->SetCraneSchedule( qc, craneSchedule );
	return;
}

void CALVManager::GetProcessIDListOfQC( int qc, deque<string>& jobIDList )
{
	m_QCs[qc].GetProcessIDList(jobIDList);
	return;
}

void CALVManager::GetProcessIDListOfATC( int atc, deque<string>& jobIDList )
{
	m_ATCs[atc].GetProcessIDList(jobIDList);
	return;
}

void CALVManager::AssignJob(string equipID, CALVJob* pALVJob)
{ 
	m_pALVEmulator->SetJobToALV(equipID, pALVJob); 
}

bool CALVManager::CheckKeepingSchedule(CALVJob* pALVJob)
{
	return m_QCs[m_QCIDList[pALVJob->QCID]].CheckKeepingSchedule(pALVJob->jobID);
}

bool CALVManager::CheckBlockHPReserved( string blockID, string hpID )
{
	return m_ATCs[m_ATCIDList[blockID]].CheckPairHPReservation(hpID);
}
