#include "StdAfx.h"
#include "ALVScheduler.h"

#define GETPROCESSID(x) atoi((x).substr(2,8).c_str())
#define POPULATION_SIZE 50

CALVScheduler::CALVScheduler(CALVManager* pALVManager)
:m_pALVManager(pALVManager), m_DispatchRule(g_SimulationSpec.dispType)
{
}

CALVScheduler::~CALVScheduler(void)
{
	m_InventoryLevel.clear();
}

//public operation--------------------------------------------------------------------------------------------
void CALVScheduler::MakeSchedule(map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob )
{
	//���� �۾��� �Ҵ��� ALV�� ã�´�.
	deque<string> fullyAvailable;
	deque<string> partiallyAvailable;

	GetAvailableALV( ALVInfo, assignedJob, unassignedJob, fullyAvailable, partiallyAvailable );

	if( fullyAvailable.empty() == true && partiallyAvailable.empty() == true ){
		return;
	}
	
	if( m_DispatchRule == DR_EarliestDeadline ){		
		UsingEarliestDeadlineRule( ALVInfo,  assignedJob, unassignedJob, fullyAvailable);
	}
	else if( m_DispatchRule == DR_MinimumInventory ){		
		UsingMinimumInventoryRule( ALVInfo,  assignedJob, unassignedJob, fullyAvailable );		
	}
	else if( m_DispatchRule == DR_DualMIL ){		
		UsingDualInventoryRule( ALVInfo, assignedJob, unassignedJob, fullyAvailable);
	}
	else if( m_DispatchRule == DR_DualSTT ){
		UsingDualShortestTravelTime( ALVInfo, fullyAvailable, partiallyAvailable,assignedJob,  unassignedJob );
	}
	else if( m_DispatchRule == DR_DualSTT2 ){
		UsingDualShortestTravelTime2( ALVInfo, fullyAvailable, partiallyAvailable,assignedJob,  unassignedJob );
	}
	else if( m_DispatchRule == DR_HurdleJump ){
		UsingDualHurdleJump( ALVInfo, fullyAvailable, partiallyAvailable,assignedJob,  unassignedJob );
	}
	else if( m_DispatchRule == DR_DualPatternBased ){
		UsingPatternBasedHeuristic( ALVInfo, fullyAvailable, partiallyAvailable, unassignedJob );
	}
	else if( m_DispatchRule == DR_GRASP ){
		UsingDualGRASP( ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
	}



	return;
}

void CALVScheduler::InformJobComplete( CALVJob* pALVJob )
{
	if( m_DispatchRule == DR_MinimumInventory ){
		m_InventoryLevel[pALVJob->QCID] -= 1;
	}
	return;
}


//private operation--------------------------------------------------------------------------------------------
void CALVScheduler::GetAvailableALV( map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable, deque<string>& partiallyAvailable )
{
	//available�� ALV ã��
	map<string, CALVInfo*>::iterator indexALVInfo;
	for( indexALVInfo = ALVInfo.begin(); indexALVInfo != ALVInfo.end(); ++indexALVInfo ){
		CALVInfo* ALVStatus = indexALVInfo->second;
		int nJobCount = ALVStatus->GetCountofJob();
		if( nJobCount == 0 ){	//�Ҵ� ���� �۾��� ���ٸ� ��ȹ ��� ���Խ�Ų��.
			fullyAvailable.push_back( ALVStatus->GetID() );
		}
		else{
			CALVJob* pCurrentJob = ALVStatus->GetCurrentJob();
			if( g_SimulationSpec.dispType == DR_HurdleJump ){
				if( nJobCount == 1 ){ //���� ���� ���� �۾��� �ϳ� ���� ALV
					if( pCurrentJob->container.conType == ConType_General20ft ){
						if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TravelToDesignatedBlock ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TravelToDesignatedQC ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else{//�۾� ������ �������� ���� �ƴϴ��� 20ft �۾� �ϳ� ó�� ���̸� partial Available ALV�̴�.
							partiallyAvailable.push_back( ALVStatus->GetID() );
						}
					}
					else{
						if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TravelToDesignatedBlock ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TravelToDesignatedQC ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
					}
				}
				else if( nJobCount == 2 ){	//20ft �۾��� �� �� ��� �ִ� ����̴�.
					CALVJob* pSecondJob = ALVStatus->GetSecondJob();
					if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TravelToDesignatedBlock ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
						if( pSecondJob->pattern == JP_AIAI || pSecondJob->pattern == JP_ANAI ){
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else{
							partiallyAvailable.push_back( ALVStatus->GetID() );
						}	
					}
					else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TravelToDesignatedQC ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
						if( pSecondJob->pattern == JP_AIAI || pSecondJob->pattern == JP_ANAI ){
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else{
							partiallyAvailable.push_back( ALVStatus->GetID() );
						}	
					}
				}
			}
			else{
				if( pCurrentJob->container.conType == ConType_General40ft ){
					if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TransferAtBlock ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
						fullyAvailable.push_back( ALVStatus->GetID() );
					}
					else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TransferAtQC ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
						fullyAvailable.push_back( ALVStatus->GetID() );
					}
				}
				else if( nJobCount == 1 ){ //���� ���� ���� �۾��� �ϳ� ���� ALV
					if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TransferAtBlock ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
						fullyAvailable.push_back( ALVStatus->GetID() );
					}
					else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TransferAtQC ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
						fullyAvailable.push_back( ALVStatus->GetID() );
					}
					else{//�۾� ������ �������� ���� �ƴϴ��� 20ft �۾� �ϳ� ó�� ���̸� partial Available ALV�̴�.
						partiallyAvailable.push_back( ALVStatus->GetID() );
					}					
				}
				else if( nJobCount == 2 ){	//20ft �۾��� �� �� ��� �ִ� ����̴�.
					CALVJob* pSecondJob = ALVStatus->GetSecondJob();
					if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TransferAtBlock ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
						if( pSecondJob->pattern == JP_AIAI || pSecondJob->pattern == JP_ANAI ){
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else{
							partiallyAvailable.push_back( ALVStatus->GetID() );
						}						
					}
					else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TransferAtQC ){//�۾� ������ �������� �����̸� ��ȹ ��� ���Խ�Ų��.
						if( pSecondJob->pattern == JP_AIAI || pSecondJob->pattern == JP_ANAI ){
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else{
							partiallyAvailable.push_back( ALVStatus->GetID() );
						}
					}
				}
			}			
		}
	}
	return;
}

void CALVScheduler::SortUnassignedJob( deque<pair<int, string>>& orderedList, map<string, CALVJob*>& unassignedJob )
{
	map<string, CALVJob*>::iterator index;
	for( index = unassignedJob.begin(); index != unassignedJob.end(); ++index ){
		orderedList.push_back(make_pair(index->second->deadline, index->first));
	}
	sort( orderedList.begin(), orderedList.end() );
	return;
}

void CALVScheduler::AssignJobToALV(string equipID, CALVJob* pALVJob, map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob )
{
	//�۾� �Ҵ�
	pALVJob->assignedEquipID = equipID;//�۾��� ALV ����
	ALVInfo[equipID]->AddJob(pALVJob);//ALV�� �۾� ����

	//��� ����
	assignedJob.insert(make_pair(pALVJob->jobID, pALVJob ));
	unassignedJob.erase(pALVJob->jobID);

	//Emulator�� �˸�
	m_pALVManager->AssignJob( equipID, pALVJob );

	return;
}


//private operation for single-carrier mode --------------------------------------------------------------------------
void CALVScheduler::UsingEarliestDeadlineRule(map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable )
{
	deque<pair<int, string>> orderedList; 
	SortUnassignedJob(orderedList, unassignedJob );//unassignedJob�� deadlne�� ���� ������ �����Ѵ�.

	deque<pair<int, string>>::iterator index;
	for(index = orderedList.begin(); index != orderedList.end(); ++index ){
		if( fullyAvailable.empty() ){
			break;
		}
		AssignJobToALV(fullyAvailable[0], unassignedJob[(*index).second], ALVInfo, assignedJob, unassignedJob );
		fullyAvailable.pop_front();
	}

	return;
}

void CALVScheduler::UsingMinimumInventoryRule(map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable )
{
	deque<string>::iterator indexALV;		
	map<string,CALVJob*>::iterator indexJob;

	for( indexALV = fullyAvailable.begin(); indexALV != fullyAvailable.end(); ++indexALV ){
		if( unassignedJob.empty() ){
			break;
		}
		CALVJob* pALVJob = FindMinimumInventoryJob(unassignedJob);//�κ��丮 ������ ���� ���� ���� ã�´�.
		AssignJobToALV( *indexALV, pALVJob, ALVInfo, assignedJob, unassignedJob );
		m_InventoryLevel[pALVJob->QCID] += 1;
	}
	return;
}

CALVJob* CALVScheduler::FindMinimumInventoryJob( map<string,CALVJob*>& unassignedJob )
{
	//������ �ּҰ� �Ҵ��ϱ�
	map<string,CALVJob*>::iterator indexJob = unassignedJob.begin();
	map<string,CALVJob*>::iterator minInventoryJob = indexJob;
	CALVJob* pALVJob = indexJob->second;

	if( m_InventoryLevel.find(pALVJob->QCID) == m_InventoryLevel.end() ){
		m_InventoryLevel.insert(make_pair(pALVJob->QCID, 0));
	}

	int minLevel = m_InventoryLevel[pALVJob->QCID];
	int minDeadline = pALVJob->deadline;

	if( (int)unassignedJob.size() == 1){
		return pALVJob;
	}
	else{
		++indexJob;
	}

	//�ּҰ� ã��
	for( ; indexJob != unassignedJob.end(); ++indexJob ){
		pALVJob = indexJob->second;
		if( m_InventoryLevel.find(pALVJob->QCID) == m_InventoryLevel.end() ){
			m_InventoryLevel.insert(make_pair(pALVJob->QCID, 0));
		}
		if( minLevel > m_InventoryLevel[pALVJob->QCID] ){
			minLevel = m_InventoryLevel[pALVJob->QCID];
			minDeadline = pALVJob->deadline;
			minInventoryJob = indexJob;
		}
		else if( minLevel == m_InventoryLevel[pALVJob->QCID] ){
			if( (pALVJob->deadline > 0 ) && ((minDeadline < 0 ) || (minDeadline > pALVJob->deadline)) ){
				minLevel = m_InventoryLevel[pALVJob->QCID];
				minDeadline = pALVJob->deadline;
				minInventoryJob = indexJob;
			}
			else if( minDeadline == pALVJob->deadline ){
				if( rand()%100 > 50 ){//�κ��丮 ������ ��������� ������ ��� ���� ������
					minLevel = m_InventoryLevel[pALVJob->QCID];
					minDeadline = pALVJob->deadline;
					minInventoryJob = indexJob;
				}
			}
		}
	}

	return minInventoryJob->second;
}


//private operation for dual-carrier mode --------------------------------------------------------------------------
void CALVScheduler::UsingDualGRASP( map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob )
{
	//Step A. unassignedJob�� deadlne�� ���� ������ �����Ѵ�.
	deque<pair<int, string>> orderedList; 
	SortUnassignedJob(orderedList, unassignedJob );


	//Step B. Local Search�� ���� �ʱ��� ����
	int nAvailableALV = 2*(int)fullyAvailable.size() + (int)partiallyAvailable.size();
	vector<string> solution[POPULATION_SIZE];
	deque<string> idList;
	idList.insert(idList.begin(), fullyAvailable.begin(), fullyAvailable.end() );
	idList.insert(idList.end(), partiallyAvailable.begin(), partiallyAvailable.end() );
	GenerateInitialSolution( solution, idList, nAvailableALV, orderedList, unassignedJob, fullyAvailable, partiallyAvailable );

	//Step C. �ʱ��� ��
	//...�⺻�� ����
	int minSolution = 0;
	
	//DecodingSolution( solution[0], orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
	int minimumCost = EvaluateSolution( solution[0], orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
	
	int i, cost;
	for( i=1; i<POPULATION_SIZE; ++i){
		//DecodingSolution( solution[i], orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
		cost = EvaluateSolution( solution[i], orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
		if( cost < minimumCost ){
			minimumCost = cost;
			minSolution = i;
		}
	}
	//...�ʱ��� �� ���� ���� �� ����
	vector<string> bestSoFar;
	bestSoFar.insert(bestSoFar.begin(), solution[minSolution].begin(), solution[minSolution].end() );

	/*
	//Step D. �ݺ� Ž��
	int previousCost = minimumCost;
	while( previousCost < minimumCost ){//��Ž�� ��� �� ���� �ظ� ã�� ���Ͽ��ٸ� Ž�� ����
		GenerateNeighbor( solution, bestSoFar, idList, nAvailableALV );//�̿��� ����

		for( i=0; i<10; ++i){//�̿��� ��
			//DecodingSolution( solution[i], orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
			cost = EvaluateSolution( solution[i], orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
			if( cost < minimumCost ){
				minimumCost = cost;
				minSolution = i;
			}
		}
		if( previousCost > minimumCost ){//���� ã�� �ذ� �� ���ٸ�
			bestSoFar.clear();
			bestSoFar.insert(bestSoFar.begin(), solution[minSolution].begin(), solution[minSolution].end() );
			previousCost = minimumCost;
		}
		else{//Ž�� ����
			break;
		}
	}
	*/

	//Step E. 20ft �̾��� �۾� ó�� ���� ����
	DecideDualLoadingSequence( bestSoFar, orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
	

	//Step F. �۾� �Ҵ�
	for( i = 0; i < (int)bestSoFar.size(); ++i ){
		if( bestSoFar[i] != "" ){
			if( m_pALVManager->CheckKeepingSchedule(unassignedJob[orderedList[i].second]) == false ){
				Assert(false, "WHY");
			}
			AssignJobToALV( bestSoFar[i], unassignedJob[orderedList[i].second], ALVInfo, assignedJob, unassignedJob );
		}
	}

	return;
}

void CALVScheduler::DecideDualLoadingSequence( vector<string>& bestSoFar, deque<pair<int, string>>& orderedList, map<string,CALVInfo*>& ALVInfo, 
									deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob )
{

	map<string, bool> checkList;//�۾� ó�� ���� ���� ���θ� üũ
	int i, j;
	for( i=0; i<(int)orderedList.size(); ++i){
		checkList.insert(make_pair(orderedList[i].second, false));
	}

	CALVJob *pFirstJob, *pSecondJob;
	deque<string>::iterator where;
	for( i=0; i<(int)bestSoFar.size(); ++i){
		string equipID = bestSoFar[i];
		where = find(fullyAvailable.begin(), fullyAvailable.end(), equipID  );
		if( where != fullyAvailable.end() ){
			if( checkList[orderedList[i].second] == false ){
				checkList[orderedList[i].second] = true;//���� ���� ���� ����
				pFirstJob = unassignedJob[orderedList[i].second];
				if( pFirstJob->container.conType == ConType_General20ft ){//20ft�� ���
					for( j=i+1; j<(int)bestSoFar.size(); ++j){
						if( equipID == bestSoFar[j] ){							
							break;
						}
					}
					if( j < (int)bestSoFar.size()){
						checkList[orderedList[j].second] = true;
						pSecondJob = unassignedJob[orderedList[j].second];

						EJobPattern jobPattern = JP_Undefined;
						EstimateCostforDualLoad(ALVInfo[equipID], pFirstJob, pSecondJob, jobPattern);
						pSecondJob->pattern = jobPattern;
					}						
				}
			}
		}
		
	}
	return;
}

void CALVScheduler::GenerateInitialSolution( vector<string>* solution, deque<string>& idList, int nAvailableALV, deque<pair<int, string>>& orderedList, map<string, CALVJob*>& unassignedJob,deque<string>& fullyAvailable, deque<string>& partiallyAvailable  )
{
	//Full�� ��� 20ft �� �� �Ǵ� 40ft �ϳ� �Ҵ� ����
	//Partial�� ��� � �۾��̵� �ϳ��� �Ҵ� ����
	int i,j;
	map<string, int> assignment;
	for( i=0; i<(int)idList.size(); ++i){
		assignment.insert(make_pair(idList[i],0));
	}
	
	int solutionSize = (int)orderedList.size();//���� ����

	deque<string> checkList;
	deque<string>::iterator where;
	map<string,int>::iterator init;
	
	for(i = 0; i < POPULATION_SIZE; ++i ){
		//���� �ʱ�ȭ
		checkList.clear();
		checkList.insert(checkList.begin(), idList.begin(), idList.end());

		for( init = assignment.begin(); init != assignment.end(); ++init){
			init->second = 0;
		}

		for( j = 0; j < solutionSize; ++j ){
			if( checkList.empty() ){
				break;
			}
			string equipID = checkList[rand()%(int)checkList.size()];
			solution[i].push_back(equipID);

			//�ش� ��ġ�� �۾��� �����´�.
			CALVJob* pALVJob = unassignedJob[orderedList[j].second];
			if( pALVJob->container.conType == ConType_General20ft ){
				assignment[equipID] += 1;
			}
			else{
				assignment[equipID] += 2;
			}

			//�Ҵ��� ALV�� Full���� Partial����
			where = find( fullyAvailable.begin(), fullyAvailable.end(), equipID	);
			if( where != fullyAvailable.end() ){//Full A
				if( assignment[equipID] >= 2 ){//�� �̻� �Ҵ� �Ұ�
					where = find( checkList.begin(), checkList.end(), equipID	);
					checkList.erase(where);
				}
			}
			else{//Partial A
				if( assignment[equipID] > 0 ){//�� �̻� �Ҵ� �Ұ�
					where = find( checkList.begin(), checkList.end(), equipID	);
					checkList.erase(where);
				}
			}
		}
	}
	return;
}

void CALVScheduler::GenerateNeighbor( vector<string>* solution, vector<string>& bestSoFar, deque<string>& idList, int nAvailableALV  )
{
	//�ϴ��� Mutation�� �����Ͽ� ���� Ȯ��
	int nSize = (int)bestSoFar.size();
	int i;
	int populationSize = 10;
	for(i = 0; i < populationSize; ++i ){
		
	}
	return;
}

int CALVScheduler::EvaluateSolution( vector<string>& solution, deque<pair<int, string>>& orderedList, map<string,CALVInfo*>& ALVInfo, 
									deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob )
{
	int totalCost = 0;

	map<string, bool> checkList;//�۾��� cost ��� ���θ� üũ
	int i, j;
	for( i=0; i<(int)orderedList.size(); ++i){
		checkList.insert(make_pair(orderedList[i].second, false));
	}

	CALVJob *pFirstJob, *pSecondJob;
	deque<string>::iterator where;
	long finishTime;
	for( i=0; i<(int)solution.size(); ++i){
		string equipID = solution[i];
		where = find(fullyAvailable.begin(), fullyAvailable.end(), equipID  );
		if( where != fullyAvailable.end() ){
			if( checkList[orderedList[i].second] == false ){//cost ���
				checkList[orderedList[i].second] = true;
				pFirstJob = unassignedJob[orderedList[i].second];
				if( pFirstJob->container.conType == ConType_General40ft ){//40ft�� ���
					long currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
					if( currentTime > pFirstJob->deadline ){//�̹� �����Ͽ����� �ð��� �����ٸ�
						totalCost -= currentTime - pFirstJob->deadline;//����ġ
					}
					else{
						finishTime = currentTime + m_pALVManager->GetTravelTimefromMap(ALVInfo[equipID]->m_WPID, pFirstJob->QCID );
						if( finishTime > pFirstJob->deadline ){
							totalCost += finishTime - pFirstJob->deadline;							 
						}	
					}
				}
				else{//20ft �۾��� ��� - �ϳ� �� �Ҵ�Ǿ����� ���� ��.
					for( j=i+1; j<(int)solution.size(); ++j){
						if( equipID == solution[j] ){							
							break;
						}
					}
					if( j < (int)solution.size()){
						checkList[orderedList[j].second] = true;
						pSecondJob = unassignedJob[orderedList[j].second];

						EJobPattern jobPattern = JP_Undefined;
						int cost = EstimateCostforDualLoad(ALVInfo[equipID], pFirstJob, pSecondJob, jobPattern).first;

						long currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
						if( currentTime > pFirstJob->deadline || currentTime > pSecondJob->deadline ){
							if( currentTime - pFirstJob->deadline > currentTime - pSecondJob->deadline ){
								totalCost -= currentTime - pFirstJob->deadline;//����ġ
							}
							else{
								totalCost -= currentTime - pSecondJob->deadline;//����ġ
							}
						}
						else{
							totalCost += cost;
						}
					}
					else{
						long currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
						if( currentTime > pFirstJob->deadline ){//�̹� �����Ͽ����� �ð��� �����ٸ�
							totalCost -= currentTime - pFirstJob->deadline;//����ġ
						}
						else{
							finishTime = currentTime + m_pALVManager->GetTravelTimefromMap(ALVInfo[equipID]->m_WPID, pFirstJob->QCID );
							if( finishTime > pFirstJob->deadline ){
								totalCost += finishTime - pFirstJob->deadline;							 
							}	
						}
					}					
				}
			}
			else{
				continue;
			}
		}
		else{//Partial�� ���
			if( checkList[orderedList[i].second] == false ){//cost ���
				checkList[orderedList[i].second] = true;
				pFirstJob = ALVInfo[equipID]->GetCurrentJob();		//������ �Ҵ�� �۾��� �����´�.
				pSecondJob = unassignedJob[orderedList[i].second];	//���� �Ҵ��� �۾��� �����´�.
				EJobPattern jobPattern = JP_Undefined;
				totalCost += EstimateCostforDualLoad( ALVInfo[equipID], pFirstJob, pSecondJob, jobPattern ).first;
			}
			else{
				continue;
			}
		}
	}
	return totalCost;
}

void CALVScheduler::DecodingSolution( vector<string>& solution, deque<pair<int, string>>& orderedList, map<string,CALVInfo*>& ALVInfo, 
						 deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob )
{
	vector<string> checkList;
	checkList.insert(checkList.begin(), fullyAvailable.begin(), fullyAvailable.end() );
	checkList.insert(checkList.end(), partiallyAvailable.begin(), partiallyAvailable.end() );

	vector<string>::iterator where;
	deque<string>::iterator index;
	//�Ҵ�� Fully A�� ���Ͽ�
	for( index = fullyAvailable.begin(); index != fullyAvailable.end(); ++index ){
		where = find(solution.begin(), solution.end(), (*index) );
		if( where != solution.end() ){//���� ALV�� solution�� �����Ѵٸ�
			where = find(checkList.begin(), checkList.end(), (*index) );
			checkList.erase( where );//�ϴ� checkList���� �����.

			//Fully A�� 20ft 2�� �Ǵ� 40ft �ϳ��� �Ҵ� �����ϴ�.
			int i, nCapacity = 0;
			for( i = 0; i<(int)solution.size(); ++i){
				if( solution[i] == (*index) ){
					if( nCapacity >= 2 ){//�Ҵ緮 ä���� �����̸�
						if( checkList.empty() ){
							solution[i] = "";
						}
						else{
							solution[i] = checkList[rand()%(int)checkList.size()];
						}
					}
					else{
						//�ش� ��ġ�� �۾��� �����´�.
						CALVJob* pALVJob = unassignedJob[orderedList[i].second];
						if( pALVJob->container.conType == ConType_General20ft ){
							nCapacity += 1;
						}
						else{
							nCapacity += 2;
						}
					}
				}				
			}
		}		
	}
	//�Ҵ�� Partially A�� ���Ͽ�
	for( index = partiallyAvailable.begin(); index != partiallyAvailable.end(); ++index ){
		where = find(solution.begin(), solution.end(), (*index) );
		if( where != solution.end() ){//���� ALV�� solution�� �����Ѵٸ�
			where = find(checkList.begin(), checkList.end(), (*index) );
			checkList.erase( where );//�ϴ� checkList���� �����.

			//Partially A�� 20ft �ϳ� �Ǵ� 40ft �ϳ��� �Ҵ� �����ϴ�.
			int i, nJob = 0;
			for( i = 0; i<(int)solution.size(); ++i){
				if( solution[i] == (*index) ){
					if( nJob >= 1 ){//�ι�° �۾����� ALV ��ü
						if( checkList.empty() ){
							solution[i] = "";
						}
						else{
							solution[i] = checkList[rand()%(int)checkList.size()];
						}
					}
					else{
						++nJob;
					}
				}				
			}
		}		
	}

	//�ʱ� ��ȹ�� ���ԵǾ����� �ʴ� ALV�� ���Ͽ� �˻�
	vector<string>::iterator cindex;
	deque<string>::iterator cwhere;
	for( cindex = checkList.begin(); cindex != checkList.end(); ++cindex ){
		where = find(solution.begin(), solution.end(), (*cindex) );
		if( where != solution.end() ){//���� ALV�� solution�� �����Ѵٸ�
			//���� Full Partial Ȯ��
			cwhere = find(fullyAvailable.begin(), fullyAvailable.end(), (*cindex) );
			if( cwhere != fullyAvailable.end() ){//Full�̸�
				//Fully A�� 20ft 2�� �Ǵ� 40ft �ϳ��� �Ҵ� �����ϴ�.
				int i, nCapacity = 0;
				for( i = 0; i<(int)solution.size(); ++i){
					if( solution[i] == (*cindex) ){
						if( nCapacity >= 2 ){//�Ҵ緮 ä���� �����̸�
							solution[i] = "";
						}
						else{
							//�ش� ��ġ�� �۾��� �����´�.
							CALVJob* pALVJob = unassignedJob[orderedList[i].second];
							if( pALVJob->container.conType == ConType_General20ft ){
								nCapacity += 1;
							}
							else{
								nCapacity += 2;
							}
						}
					}				
				}	
			}
			else{//Partial�̸�
				int i, nJob = 0;
				for( i = 0; i<(int)solution.size(); ++i){
					if( solution[i] == (*cindex) ){
						if( nJob >= 1 ){//�ι�° �۾����� ALV ��ü
							solution[i] = "";
						}
						else{
							++nJob;
						}
					}				
				}
			}
		}
	}

	return;
}




void CALVScheduler::UsingDualHurdleJump( map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob )
{
	deque<pair<int, string>> orderedList; 
	SortUnassignedJob(orderedList, unassignedJob );//unassignedJob�� deadlne�� ���� ������ �����Ѵ�.

	deque<pair<int, string>>::iterator indexList;
	deque<string>::iterator where;
	while( orderedList.empty() == false ){
		indexList = orderedList.begin();
		
		if( fullyAvailable.empty() && partiallyAvailable.empty() ){
			break;
		}

		//�ش� �۾��� ���Ͽ� ALV�� �۾��� ���� ������ ������ ����Ѵ�.]
		CALVJob* pNewALVJob = unassignedJob[indexList->second];
		if( pNewALVJob->prsType == PrsType_Loading ){
			//���� �۾��� ���/QC�� ��ȹ �� ������ ���� ���� �۾��� ������ �Ҵ���� �ʾҴٸ� PASS
			if( m_pALVManager->CheckKeepingSchedule(pNewALVJob) == false ){
				orderedList.erase(indexList);
				continue;
			}
		}
		/*
		//�Ʒ��� ���� ó���� ��� 40ft �۾��� ���� ó�� ����
		if( pNewALVJob->container.conType == ConType_General40ft && fullyAvailable.empty() == true ){//40ft�۾� ���� �Ұ� ó��
			orderedList.erase(indexList);
			continue;
		}
		*/

		deque<pair<int, string>> sortedALVList;
		SortALVList(sortedALVList, pNewALVJob, ALVInfo, fullyAvailable, partiallyAvailable);//40ft�۾��� ��� �� �Լ� �ȿ��� partial ����

		
		deque<pair<int, string>>::iterator indexALV= sortedALVList.begin();
		//Fully�̰� 20ft �۾��� ��쿡�� Best ����
		where = find( fullyAvailable.begin(), fullyAvailable.end(), indexALV->second );
		if( where != fullyAvailable.end() && pNewALVJob->container.conType == ConType_General20ft ){
			AssignJobToALV( indexALV->second, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
		}
		else{
			//40ft �۾��̰ų� Partially�� ��쿡�� Hurdle ����
			if( indexALV->first >= pNewALVJob->deadline ){//ù��° ALV�� �̹� ������� ���� ���� �Ұ���� �ٷ� ����
				AssignJobToALV( indexALV->second, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
			}
			else{
				bool assigned = false;
				for( ; indexALV != sortedALVList.end(); ++indexALV ){
					if( indexALV->first >= pNewALVJob->deadline ){//������� ��� ���Ѵٸ� ���� �۾��� �Ҵ�
						--indexALV;
						AssignJobToALV( indexALV->second, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
						assigned = true;
						break;
					}
				}
				if( assigned == false ){//��� ������ ������� ��� ������ ����̴�.
					--indexALV;
					AssignJobToALV( indexALV->second, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
				}
			}
		}
		//�۾� ����
		orderedList.erase(indexList);
		//ALV ����
		where = find( fullyAvailable.begin(), fullyAvailable.end(), indexALV->second );
		if( where != fullyAvailable.end() ){
			string equipID = (*where);
			fullyAvailable.erase(where);

			if( pNewALVJob->container.conType == ConType_General20ft ){//fullyAvailable�� 20ft �۾��� �Ҵ��� ���
				//vehicle-initiated Rule�� 20ft �۾� �ϳ� �� �Ҵ�
				CALVJob* pAssignedJob = pNewALVJob;//��ȹ �߿� �Ҵ�� �۾��� AssignedJob
				EJobPattern jobPattern = JP_Undefined, shortestJobPattern;
				deque<pair<int, string>>::iterator shortestJob = orderedList.end();
				pair<int,int> lowestCost, cost;
				lowestCost = make_pair(10000,10000);
				for( indexList = orderedList.begin(); indexList != orderedList.end(); ++indexList ){
					pNewALVJob = unassignedJob[indexList->second];
					if( pNewALVJob->container.conType == ConType_General20ft ){
						cost = EstimateCostforDualLoad(ALVInfo[equipID], pAssignedJob, pNewALVJob, jobPattern );
						if( lowestCost.first > cost.first ){
							lowestCost = cost;
							shortestJob = indexList;
							shortestJobPattern = jobPattern;
						}						
					}
					else{//40ft�۾� ���� �Ұ�
						continue;
					}
				}
				if( shortestJob != orderedList.end() ){//�۾� �Ҵ�� ���
					unassignedJob[shortestJob->second]->pattern = shortestJobPattern;
					AssignJobToALV( equipID, unassignedJob[shortestJob->second], ALVInfo, assignedJob, unassignedJob );
					orderedList.erase(shortestJob);
				}
			}			
		}
		else{
			where = find( partiallyAvailable.begin(), partiallyAvailable.end(), indexALV->second );
			if( where != partiallyAvailable.end() ){
				partiallyAvailable.erase(where);
			}
		}

		
	}



	return;
}

void CALVScheduler::SortALVList( deque<pair<int, string>>& sortedALVList, CALVJob* pALVJob, map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable )
{
	deque<string>::iterator index;
	long estimatedTime;
	for( index = fullyAvailable.begin(); index != fullyAvailable.end(); ++index ){
		estimatedTime = m_pALVManager->GetTravelTimefromMap(ALVInfo[*index]->m_WPID, pALVJob->QCID );
		sortedALVList.push_back( make_pair(estimatedTime, *index) );
	}
	//if( pALVJob->container.conType == ConType_General20ft )
	{
		for( index = partiallyAvailable.begin(); index != partiallyAvailable.end(); ++index ){
			CALVJob* pAssignedJob;//������ �Ҵ�� �۾��� �����´�.
			if( ALVInfo[*index]->GetCountofJob() >= 2 ){//��ȹ �߿� �Ҵ�� �۾��� �ִ� ���
				pAssignedJob = ALVInfo[*index]->GetSecondJob();
			}
			else{//��ȹ ���� �Ҵ�� �۾��� �ִ� ���
				pAssignedJob = ALVInfo[*index]->GetCurrentJob();//������ �Ҵ�� �۾��� ã�´�.
			}
			estimatedTime = m_pALVManager->GetTravelTimeEstimated(*index, pAssignedJob);
			estimatedTime += m_pALVManager->GetTravelTimefromMap(pAssignedJob->QCID, pALVJob->QCID);
			sortedALVList.push_back( make_pair(estimatedTime, *index) );
		}
	}
	
	sort( sortedALVList.begin(), sortedALVList.end() );

	return;
}

void CALVScheduler::UsingDualShortestTravelTime2(map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob )
{
	int travelTime, shortestTravelTime;
	map<string,CALVJob*>::iterator shortestJob;
	map<string,CALVJob*>::iterator indexJob;

	deque<string>::iterator indexALV;
	while( fullyAvailable.empty() == false ){
		if( unassignedJob.empty() == true ){
			break;
		}

		indexALV = fullyAvailable.begin();
		shortestTravelTime = 10000;
		for( indexJob = unassignedJob.begin(); indexJob != unassignedJob.end(); ++indexJob ){
			travelTime = m_pALVManager->GetTravelTimefromMap(ALVInfo[*indexALV]->m_WPID, indexJob->second->QCID );
			if( shortestTravelTime > travelTime ){
				shortestTravelTime = travelTime;
				shortestJob = indexJob;
			}
		}
		if( shortestJob->second->container.conType == ConType_General20ft ){
			partiallyAvailable.push_back(*indexALV);
		}
		AssignJobToALV( *indexALV, shortestJob->second, ALVInfo, assignedJob, unassignedJob );
		fullyAvailable.pop_front();
	}


	while( partiallyAvailable.empty() == false ){//20ft �۾� �Ҵ�
		if( unassignedJob.empty() == true ){
			break;
		}
		indexALV = partiallyAvailable.begin();
		CALVJob* pAssignedJob;//������ �Ҵ�� �۾��� ã�´�.
		if( ALVInfo[*indexALV]->GetCountofJob() >= 2 ){//��ȹ �߿� �Ҵ�� �۾��� �ִ� ���
			pAssignedJob = ALVInfo[*indexALV]->GetSecondJob();
		}
		else{//��ȹ ���� �Ҵ�� �۾��� �ִ� ���
			pAssignedJob = ALVInfo[*indexALV]->GetCurrentJob();//������ �Ҵ�� �۾��� ã�´�.
		}
		//travelTime = m_pALVManager->GetTravelTimeEstimated(*indexALV, pAssignedJob );//������ ����. ���� ����� �ʿ� ����

		shortestTravelTime = 10000;
		EJobPattern jobPattern = JP_Undefined;
		for( indexJob = unassignedJob.begin(); indexJob != unassignedJob.end(); ++indexJob ){
			if( indexJob->second->container.conType == ConType_General20ft ){				
				travelTime = m_pALVManager->GetTravelTimefromMap(pAssignedJob->QCID, indexJob->second->QCID);
				if( shortestTravelTime > travelTime ){
					shortestTravelTime = travelTime;
					shortestJob = indexJob;
				}
			}
		}
		if( shortestTravelTime != 10000 ){
			EstimateCostforDualLoad(ALVInfo[*indexALV], pAssignedJob, shortestJob->second, jobPattern );
			shortestJob->second->pattern = jobPattern;
			AssignJobToALV( *indexALV, shortestJob->second, ALVInfo, assignedJob, unassignedJob );
			partiallyAvailable.pop_front();
		}
		else{//partially Available�� ���� ������ �۾��� ����
			break;
		}
	}
	
	
	return;
}



void CALVScheduler::UsingDualShortestTravelTime(map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob )
{
	deque<pair<int, string>> orderedList; 
	SortUnassignedJob(orderedList, unassignedJob );//unassignedJob�� deadlne�� ���� ������ �����Ѵ�.

	//�� �۾��� ���Ͽ� shorteset travel time�� ������ ALV�� �Ҵ��Ѵ�.
	deque<string>::iterator partialIndex, fullIndex, shortestIndex;
	int travelTime, shortestTravelTime;

	deque<pair<int, string>>::iterator index;//deque<(deadline,JobID)> ����
	for(index = orderedList.begin(); index != orderedList.end(); ++index ){
		if( fullyAvailable.empty() && partiallyAvailable.empty() ){
			break;
		}

		CALVJob* pNewALVJob = unassignedJob[index->second];

		//�ʱ��ظ� ���Ѵ�.
		bool partialALV = false;//�Ҵ�� ALV�� � ����� ALV���� üũ
		if( fullyAvailable.empty() == false ){
			shortestIndex = fullyAvailable.begin();
			shortestTravelTime = m_pALVManager->GetTravelTimefromMap(ALVInfo[*shortestIndex]->m_WPID, pNewALVJob->QCID );
		}
		else{
			if( pNewALVJob->container.conType != ConType_General20ft ){
				continue;//40ft �۾��� ������ �� �ִ� ALV�� ���� ���
			}
			shortestIndex = partiallyAvailable.begin();
			CALVJob* pAssignedJob;//������ �Ҵ�� �۾��� ã�´�.
			if( ALVInfo[*shortestIndex]->GetCountofJob() >= 2 ){//��ȹ �߿� �Ҵ�� �۾��� �ִ� ���
				pAssignedJob = ALVInfo[*shortestIndex]->GetSecondJob();
			}
			else{//��ȹ ���� �Ҵ�� �۾��� �ִ� ���
				pAssignedJob = ALVInfo[*shortestIndex]->GetCurrentJob();//������ �Ҵ�� �۾��� ã�´�.
			}			
			shortestTravelTime = m_pALVManager->GetTravelTimeEstimated(*shortestIndex, pAssignedJob);
			shortestTravelTime += m_pALVManager->GetTravelTimefromMap(pAssignedJob->QCID, pNewALVJob->QCID);
			partialALV = true;
		}

		if( fullyAvailable.empty() == false ){//���� fully-available ALV �߿��� STT�� ������ ���� ã�´�.
			for( fullIndex = fullyAvailable.begin(); fullIndex != fullyAvailable.end(); ++fullIndex ){
				travelTime = m_pALVManager->GetTravelTimefromMap(ALVInfo[*fullIndex]->m_WPID, pNewALVJob->QCID );
				if( shortestTravelTime > travelTime ){
					shortestTravelTime = travelTime;
					shortestIndex = fullIndex;
					partialALV = false;
				}
			}
		}		
		if( partiallyAvailable.empty() == false && pNewALVJob->container.conType == ConType_General20ft){//Ž�� ����� partially available�� ALV�� Ȯ���Ѵ�.
			for( partialIndex = partiallyAvailable.begin(); partialIndex != partiallyAvailable.end(); ++partialIndex ){
				CALVJob* pAssignedJob;//������ �Ҵ�� �۾��� ã�´�.
				if( ALVInfo[*partialIndex]->GetCountofJob() >= 2 ){//��ȹ �߿� �Ҵ�� �۾��� �ִ� ���
					pAssignedJob = ALVInfo[*partialIndex]->GetSecondJob();
				}
				else{//��ȹ ���� �Ҵ�� �۾��� �ִ� ���
					pAssignedJob = ALVInfo[*partialIndex]->GetCurrentJob();//������ �Ҵ�� �۾��� ã�´�.
				}
				travelTime = m_pALVManager->GetTravelTimeEstimated(*partialIndex, pAssignedJob );
				travelTime += m_pALVManager->GetTravelTimefromMap(pAssignedJob->QCID, pNewALVJob->QCID);
				if( shortestTravelTime >= travelTime ){
					shortestTravelTime = travelTime;
					shortestIndex = partialIndex;
					partialALV = true;
				}
			}
		}

		if( partialALV ){//�� �߿��� �Ҵ�Ǿ��ٸ�
			pNewALVJob->pattern = JP_ANAN;
			AssignJobToALV( *shortestIndex, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
			partiallyAvailable.erase(shortestIndex);
		}
		else{//full ALV���� �Ҵ�Ǿ��ٸ�
			if( unassignedJob[index->second]->container.conType == ConType_General20ft ){
				partiallyAvailable.push_back(*shortestIndex);
			}	
			AssignJobToALV( *shortestIndex, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
			fullyAvailable.erase(shortestIndex);
		}
	}

	return;
}



void CALVScheduler::UsingDualInventoryRule(map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable)
{
	deque<string>::iterator indexALV;		
	map<string,CALVJob*>::iterator indexJob;

	for( indexALV = fullyAvailable.begin(); indexALV != fullyAvailable.end(); ++indexALV ){
		if( unassignedJob.empty() ){
			break;
		}
		//20x20 �۾��� Dual-loading
		CALVJob* pALVJob = FindMinimumInventoryJob(unassignedJob);//�κ��丮 ������ ���� ���� ���� ã�´�.
		AssignJobToALV( *indexALV, pALVJob, ALVInfo, assignedJob, unassignedJob );
		if( pALVJob->container.conType == ConType_General20ft ){
			if( pALVJob->connectedJobList.empty() == false ){//20x20 �۾��� ���
				vector<pair<ERelativePosition, string>>::iterator index;
				map<string,CALVJob*>::iterator indexJob;
				for( index = pALVJob->connectedJobList.begin(); index != pALVJob->connectedJobList.end(); ++index ){
					if( pALVJob->relativePosition == ERP_Left && index->first == ERP_Right ){
						break;
					}
					else if(  pALVJob->relativePosition == ERP_Right && index->first == ERP_Left ){
						break;
					}
				}
				if( index != pALVJob->connectedJobList.end() ){//�ش� �۾��� unassignedJob ��Ͽ� �ִ��� Ȯ���Ѵ�.
					for( indexJob = unassignedJob.begin(); indexJob != unassignedJob.end(); ++indexJob ){
						if( indexJob->second->jobID == index->second ){//�۾� �Ҵ�
							//�۾� Ÿ�� ����
							if( indexJob->second->prsType == PrsType_Discharging ){
								indexJob->second->pattern = JP_AIAN;
							}
							else{
								indexJob->second->pattern = JP_ANAI;
							}							
							AssignJobToALV( *indexALV, indexJob->second, ALVInfo, assignedJob, unassignedJob );
							break;
						}
					}
				}
			}
		}		
		m_InventoryLevel[pALVJob->QCID] += 1;
	}
	return;
	
	return;
}
void CALVScheduler::UsingPatternBasedHeuristic( map<string,CALVInfo*> ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable, map<string, CALVJob*> unassignedJob )
{
	deque<string>::iterator emptyALV;
	deque<string>::iterator partialALV;
	map<string,CALVJob*>::iterator indexJob;

	map<string, CALVJob*> assignedJob;//��ȹ �� �Ҵ��� �۾��� ���Ͽ� dual-load�� �����ϱ� ����

	while( unassignedJob.empty() == false ){//���Ҵ�� �۾��� ���� ������ �ݺ�
		if(  fullyAvailable.empty() ){//���� ALV�� ���� ������ �ݺ�
			if( !partiallyAvailable.empty() ){//20ft �۾� ���ุ ������ ALV
				bool check20ftJob = false;
				for( indexJob = unassignedJob.begin(); indexJob != unassignedJob.end(); ++indexJob ){
					if( indexJob->second->container.conType == ConType_General20ft  ){
						check20ftJob = true;
						break;
					}
				}
				if( check20ftJob == false ){
					break;
				}
			}
			else{
				break;
			}			
		}
		

		//Step.A. ���Ҵ�� �۾� �� earliest deadline�� ã�´�.
		int earliestDeadline = -1;
		map<string,CALVJob*>::iterator earliestDeadlineJob = unassignedJob.begin();
		for( indexJob = unassignedJob.begin(); indexJob != unassignedJob.end(); ++indexJob ){
			if( earliestDeadline == -1 && indexJob->second->deadline > 0  ){//�ʱ�ȭ
				earliestDeadline = indexJob->second->deadline;
				earliestDeadlineJob = indexJob;
			}
			else if( earliestDeadline > indexJob->second->deadline ){//������� ��
				earliestDeadline = indexJob->second->deadline;
				earliestDeadlineJob = indexJob;
			}
			else if( earliestDeadline == indexJob->second->deadline ){//deadline�� �����ϴٸ�
				if( earliestDeadlineJob->second->prsType == PrsType_Loading && indexJob->second->prsType == PrsType_Loading ){//��� �����̸�
					if( GETPROCESSID(earliestDeadlineJob->second->jobID) > GETPROCESSID(indexJob->second->jobID) ){//������ ���� ���� ����
						earliestDeadline = indexJob->second->deadline;
						earliestDeadlineJob = indexJob;
					}
				}
				else{
					if( earliestDeadlineJob->second->prsType != PrsType_Loading && indexJob->second->prsType == PrsType_Loading ){//���ϸ� �켱
						earliestDeadline = indexJob->second->deadline;
						earliestDeadlineJob = indexJob;
					}
				}
			}
		}

		//Step.B. ALV�� �۾� �Ҵ� �� ����Ǵ� QC ������ �����Ѵ�.
		pair<int,int> lowestCost;
		deque<string>::iterator lowestCostALV;
		EJobPattern JobPatternDetermined = JP_Undefined;

		//�ʱⰪ ���ϱ�
		if( fullyAvailable.empty() == false ){
			lowestCostALV = fullyAvailable.begin();			
			lowestCost = EstimateCost( ALVInfo[*lowestCostALV], earliestDeadlineJob->second );
		}
		else{
			lowestCostALV = partiallyAvailable.begin();

			CALVJob* pCurrentJob;			

			//��ȹ �߿� �Ҵ�� �۾� ã��
			map<string, CALVJob*>::iterator index;
			for( index = assignedJob.begin(); index != assignedJob.end(); ++index ){
				if( *lowestCostALV == index->first ){
					pCurrentJob = index->second;
					break;
				}
			}
			//��ȹ ���� �Ҵ�Ǿ��� �۾��� ���
			if( index == assignedJob.end() ){	
				pCurrentJob = ALVInfo[*lowestCostALV]->GetCurrentJob();
			}

			EJobPattern JobPattern = JP_Undefined;
			lowestCost = EstimateCostforDualLoad( ALVInfo[*lowestCostALV], pCurrentJob, earliestDeadlineJob->second, JobPattern );
			JobPatternDetermined = JobPattern;
		}

		//�ּҰ� ã��
		pair<int,int> cost;
		if( fullyAvailable.empty() == false ){			
			for( emptyALV = fullyAvailable.begin(); emptyALV != fullyAvailable.end(); ++emptyALV ){
				cost = EstimateCost( ALVInfo[*emptyALV], earliestDeadlineJob->second );
				if( lowestCost.first > cost.first ){
					lowestCost = cost;
					lowestCostALV = emptyALV;
				}
				else if( lowestCost.first == cost.first ){
					if( lowestCost.second > cost.second ){
						lowestCost = cost;
						lowestCostALV = emptyALV;
					}
				}
			}
		}
		if( partiallyAvailable.empty() == false ){
			if( earliestDeadlineJob->second->container.conType == ConType_General20ft ){//�� ��� Partially Available�� ���
				for( partialALV = partiallyAvailable.begin(); partialALV != partiallyAvailable.end(); ++ partialALV ){
					//�Ҵ�� �۾� ã��
					CALVJob* pCurrentJob;			
					map<string, CALVJob*>::iterator index;
					for( index = assignedJob.begin(); index != assignedJob.end(); ++index ){
						if( *partialALV == index->first ){
							pCurrentJob = index->second;
							break;
						}
					}
					if( index == assignedJob.end() ){	//��ȹ ���� �Ҵ�Ǿ��� �۾��� ���
						pCurrentJob = ALVInfo[*partialALV]->GetCurrentJob();
					}

					EJobPattern JobPattern = JP_Undefined;
					cost = EstimateCostforDualLoad( ALVInfo[*partialALV], pCurrentJob, earliestDeadlineJob->second, JobPattern );
					if( lowestCost.first > cost.first ){
						lowestCost = cost;
						lowestCostALV = partialALV;
						JobPatternDetermined = JobPattern;
					}
					else if( lowestCost.first == cost.first ){
						if( lowestCost.second > cost.second ){
							lowestCost = cost;
							lowestCostALV = partialALV;
							JobPatternDetermined = JobPattern;
						}
					}
				}
			}
		}


		//Step.C. Lowest assignment cost�� ������ ���� ����, 
		//m_Schedule.push_back( make_pair(*lowestCostALV, earliestDeadlineJob->first) );	//ALVID JobID
		assignedJob.insert( make_pair( *lowestCostALV, earliestDeadlineJob->second) );

		//Available ALV ��� ����
		if( earliestDeadlineJob->second->container.conType == ConType_General20ft ){
			emptyALV = find( fullyAvailable.begin(), fullyAvailable.end(), *lowestCostALV);
			if( emptyALV != fullyAvailable.end() ){	// Empty ALV�� 20ft �۾��� �Ҵ��ߴٸ� --> partially available ��Ͽ� �߰� ��, fully���� ����
				partiallyAvailable.push_back(*lowestCostALV);
				fullyAvailable.erase( emptyALV );
			}
			else{
				partialALV = find( partiallyAvailable.begin(), partiallyAvailable.end(), *lowestCostALV);
				if( partialALV != partiallyAvailable.end() ){
					partiallyAvailable.erase( partialALV );

					//20x20ft �۾��� �� ����̹Ƿ� �۾� ���� ������ ��������
					earliestDeadlineJob->second->pattern = JobPatternDetermined;
				}
			}
		}
		else{//Empty�� 40ft �Ҵ��� �����
			emptyALV = find( fullyAvailable.begin(), fullyAvailable.end(), *lowestCostALV);
			if( emptyALV != fullyAvailable.end() ){
				fullyAvailable.erase( emptyALV );
			}
		}
		unassignedJob.erase(earliestDeadlineJob);										//unassigned order ����


	}

	//InformSchedule();
	return;
}

pair<int,int> CALVScheduler::EstimateCost( CALVInfo* pALVInfo, CALVJob* pALVJob ){
	int cost = 0;
	int travelTime = 0;
	int estimatedFinishTime;
	if( pALVJob->prsType == PrsType_Discharging	 ){//������ ��� QC���� ���� �͸� ���
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pALVJob->QCID + "Q0" );		
	}
	else if( pALVJob->prsType == PrsType_Loading ){//������ ��� Block���� ���� �ð� + QC�� ���� �ð� ���
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pALVJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pALVJob->BlockID + "S0", pALVJob->QCID + "Q0" );		
	}

	estimatedFinishTime = travelTime + (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);//���� �ð� + ���࿡ �ʿ��� �ð�
	if( estimatedFinishTime < pALVJob->deadline ){
		cost = 0; //QC ������ �߻����� �ʾҴ�.				
	}
	else{
		cost = estimatedFinishTime - pALVJob->deadline;
	}

	return make_pair(cost, travelTime);
}

pair<int,int> CALVScheduler::EstimateCostforDualLoad( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob, EJobPattern& JobPattern )
{
	//AANN, ANAN, ANNA ������ ����Ͽ� cost�� ���� ���� ����	
	pair<int, int> lowestCost;	

	//ANN�� ������ ���
	if( pAssignedJob->prsType == PrsType_Discharging && ( pAssignedJob->jobState == JobState_TravelToYardside || pAssignedJob->jobState == JobState_TravelToDesignatedBlock) ){
		//���� �۾��� Drop-off�� �����Ϸ� ���� ��
		lowestCost = GetANNcost( pALVInfo, pAssignedJob, pNewJob );
		JobPattern = JP_AANN;
		return lowestCost;

	}
	else if( pAssignedJob->prsType == PrsType_Loading && (pAssignedJob->jobState == JobState_TravelToQuayside || pAssignedJob->jobState == JobState_TravelToDesignatedQC) ){
		//���� �۾��� Drop-off�� �����Ϸ� ���� ��
		lowestCost = GetANNcost( pALVInfo, pAssignedJob, pNewJob );
		JobPattern = JP_AANN;
		return lowestCost;
	}

	if( pNewJob->container.conType == ConType_General40ft ){
		lowestCost = GetAANNcost( pALVInfo, pAssignedJob, pNewJob );
		JobPattern = JP_AANN;
		return lowestCost;
	}


	pair<int,int> cost;
	if( pAssignedJob->prsType == PrsType_Loading && pNewJob->prsType ==PrsType_Discharging ){
		//AANN
		lowestCost = GetAANNcost( pALVInfo, pAssignedJob, pNewJob );
		JobPattern = JP_AANN;
	}
	else if( pAssignedJob->prsType == PrsType_Loading && pNewJob->prsType ==PrsType_Discharging ){
		//AANN
		lowestCost = GetAANNcost( pALVInfo, pAssignedJob, pNewJob );
		JobPattern = JP_AANN;
	}
	else{
		//ANAN
		lowestCost = GetANANcost( pALVInfo, pAssignedJob, pNewJob );
		JobPattern = JP_ANAN;
	}
	
	if( JobPattern != JP_ANAN ){
		//ANAN
		cost = GetANANcost( pALVInfo, pAssignedJob, pNewJob );
		if( lowestCost.first > cost.first ){
			lowestCost = cost;
			JobPattern = JP_ANAN;
		}
		else if( lowestCost.first == cost.first ){
			if( lowestCost.second > cost.second ){
				lowestCost = cost;
				JobPattern = JP_ANAN;
			}
		}
	}

	//ANNA
	cost = GetANNAcost( pALVInfo, pAssignedJob, pNewJob );
	if( lowestCost.first > cost.first ){
		lowestCost = cost;
		JobPattern = JP_ANAN;
	}
	else if( lowestCost.first == cost.first ){
		if( lowestCost.second > cost.second ){
			lowestCost = cost;
			JobPattern = JP_ANAN;
		}
	}

	bool incorporatable = false;
	if( pAssignedJob->prsType == PrsType_Discharging && pNewJob->prsType == PrsType_Discharging ){
		incorporatable = true;
	}
	else if( pAssignedJob->prsType == PrsType_Loading && pNewJob->prsType == PrsType_Loading ){
		incorporatable = true;
	}

	if( incorporatable ){
		//AI�� ���
		if( pAssignedJob->prsType == PrsType_Discharging && pAssignedJob->CheckConnectedJob(pNewJob->jobID) ){
			//AIAI, AINA, AIAN ����
			if( pAssignedJob->BlockID == pNewJob->BlockID ){			//AIAI
				cost = GetAIAIcost( pALVInfo, pAssignedJob, pNewJob );
				if( lowestCost.first > cost.first ){
					lowestCost = cost;
					JobPattern = JP_AIAI;
				}
			}
			else{
				//AIAN
				cost = GetAIANcost( pALVInfo, pAssignedJob, pNewJob );
				if( lowestCost.first > cost.first ){
					lowestCost = cost;
					JobPattern = JP_AIAN;
				}
				else if( lowestCost.first == cost.first ){
					if( lowestCost.second > cost.second ){
						lowestCost = cost;
						JobPattern = JP_AIAN;
					}
				}

				//AINA
				cost = GetAINAcost( pALVInfo, pAssignedJob, pNewJob );
				if( lowestCost.first > cost.first ){
					lowestCost = cost;
					JobPattern = JP_AINA;
				}
				else if( lowestCost.first == cost.first ){
					if( lowestCost.second > cost.second ){
						lowestCost = cost;
						JobPattern = JP_AINA;
					}
				}
			}

		}
		else if( pAssignedJob->prsType == PrsType_Loading && pAssignedJob->CheckConnectedJob(pNewJob->jobID) ){
			//ANAI ����
			cost = GetANAIcost( pALVInfo, pAssignedJob, pNewJob );
			if( lowestCost.first > cost.first ){
				lowestCost = cost;
				JobPattern = JP_ANAI;
			}
			else if( lowestCost.first == cost.first ){
				if( lowestCost.second > cost.second ){
					lowestCost = cost;
					JobPattern = JP_ANAI;
				}
			}
		}
	}
	
	return lowestCost;
}

pair<int,int> CALVScheduler::GetANNcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob )
{
	int cost, travelTime;
	
	if( pAssignedJob->prsType ==PrsType_Discharging ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
	}
	else{// if( pNewJob->prsType ==PrsType_Loading ){	
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
	}	
	
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pNewJob->deadline ){//�̹� ��������� ���� ���
		cost = 0;
	}
	else{
		int estimatedFinishTime = currentTime + travelTime;
		if( estimatedFinishTime < pNewJob->deadline ){
			cost = 0;//QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost = estimatedFinishTime - pNewJob->deadline;
		}
	}

	return make_pair(cost, travelTime);
}

pair<int,int> CALVScheduler::GetAANNcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob )
{
	int cost, travelTime;
	//AA
	if( pAssignedJob->prsType == PrsType_Discharging ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pAssignedJob->BlockID + "S0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		//NN
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
	}
	else{// if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pAssignedJob->QCID + "Q0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		//NN
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
	}
	
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pNewJob->deadline ){//�̹� ��������� ���� ���
		cost = 0;
	}
	else{
		int estimatedFinishTime = currentTime + travelTime;
		if( estimatedFinishTime < pNewJob->deadline ){
			cost = 0;//QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost = estimatedFinishTime - pNewJob->deadline;
		}
	}
	return make_pair(cost, travelTime);
}


pair<int,int> CALVScheduler::GetANANcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob )
{
	int cost, travelTime, assignedTravelTime;

	//ANAN
	if( pAssignedJob->prsType == PrsType_Discharging ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
		assignedTravelTime = travelTime;
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pAssignedJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap(pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			assignedTravelTime = travelTime + m_pALVManager->GetTravelTimefromMap( pNewJob->QCID + "Q0", pAssignedJob->QCID + "Q0" );
			assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pAssignedJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			assignedTravelTime = travelTime;
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
	}
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//�̹� ��������� ���� ���
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost += estimatedFinishTime - pNewJob->deadline;
		}
	}
	return make_pair(cost, travelTime);
}

pair<int,int> CALVScheduler::GetANNAcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob )
{
	int cost, travelTime, assignedTravelTime;

	//ANAN
	if( pAssignedJob->prsType == PrsType_Discharging ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
		assignedTravelTime = travelTime;
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			assignedTravelTime = travelTime + m_pALVManager->GetTravelTimefromMap( pNewJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			assignedTravelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pAssignedJob->QCID + "Q0" );
			assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
			assignedTravelTime = travelTime + m_pALVManager->GetTravelTimefromMap( pNewJob->QCID + "Q0", pAssignedJob->QCID + "Q0" );
			assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		}
	}
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//�̹� ��������� ���� ���
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost += estimatedFinishTime - pNewJob->deadline;
		}
	}
	return make_pair(cost, assignedTravelTime);
}

pair<int,int> CALVScheduler::GetAIAIcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob )
{
	int cost, travelTime;
	if( pAssignedJob->prsType == PrsType_Discharging ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pAssignedJob->QCID + "Q0" );
	}

	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pNewJob->deadline ){//�̹� ��������� ���� ���
		cost = 0;
	}
	else{
		int estimatedFinishTime = currentTime + travelTime;
		if( estimatedFinishTime < pNewJob->deadline ){
			cost = 0;//QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost = estimatedFinishTime - pNewJob->deadline;
		}
	}
	return make_pair(cost, travelTime);
}

pair<int,int> CALVScheduler::GetAIANcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob )
{
	int cost, travelTime, assignedTravelTime;
	if( pAssignedJob->prsType == PrsType_Discharging ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
		assignedTravelTime = travelTime;
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pAssignedJob->QCID + "Q0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		assignedTravelTime = travelTime;
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
	}

	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//�̹� ��������� ���� ���
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost += estimatedFinishTime - pNewJob->deadline;
		}
	}
	return make_pair(cost, travelTime);
}

pair<int,int> CALVScheduler::GetAINAcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob )
{
	int cost, travelTime, assignedTravelTime;
	if( pAssignedJob->prsType == PrsType_Discharging ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
		assignedTravelTime = travelTime;
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		assignedTravelTime = travelTime + m_pALVManager->GetTravelTimefromMap( pNewJob->QCID + "Q0", pAssignedJob->QCID + "Q0" );
		assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
	}

	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//�̹� ��������� ���� ���
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost += estimatedFinishTime - pNewJob->deadline;
		}
	}
	return make_pair(cost, assignedTravelTime);
}

pair<int,int> CALVScheduler::GetANAIcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob )
{
	int cost, travelTime, assignedTravelTime;
	if( pAssignedJob->prsType == PrsType_Discharging ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
		assignedTravelTime = travelTime;
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0");
		travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//��� HP ��� �ð� (���� ����غ� ��)
		assignedTravelTime = travelTime;
	}
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//�̹� ��������� ���� ���
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC ������ �߻����� �ʾҴ�.				
		}
		else{
			cost += estimatedFinishTime - pNewJob->deadline;
		}
	}
	return make_pair(cost, travelTime);
}


