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
	//먼저 작업을 할당할 ALV를 찾는다.
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
	//available한 ALV 찾기
	map<string, CALVInfo*>::iterator indexALVInfo;
	for( indexALVInfo = ALVInfo.begin(); indexALVInfo != ALVInfo.end(); ++indexALVInfo ){
		CALVInfo* ALVStatus = indexALVInfo->second;
		int nJobCount = ALVStatus->GetCountofJob();
		if( nJobCount == 0 ){	//할당 받은 작업이 없다면 계획 대상에 포함시킨다.
			fullyAvailable.push_back( ALVStatus->GetID() );
		}
		else{
			CALVJob* pCurrentJob = ALVStatus->GetCurrentJob();
			if( g_SimulationSpec.dispType == DR_HurdleJump ){
				if( nJobCount == 1 ){ //현재 수행 중인 작업이 하나 뿐인 ALV
					if( pCurrentJob->container.conType == ConType_General20ft ){
						if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TravelToDesignatedBlock ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TravelToDesignatedQC ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else{//작업 수행이 끝나가는 중이 아니더라도 20ft 작업 하나 처리 중이면 partial Available ALV이다.
							partiallyAvailable.push_back( ALVStatus->GetID() );
						}
					}
					else{
						if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TravelToDesignatedBlock ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TravelToDesignatedQC ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
					}
				}
				else if( nJobCount == 2 ){	//20ft 작업을 두 개 들고 있는 경우이다.
					CALVJob* pSecondJob = ALVStatus->GetSecondJob();
					if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TravelToDesignatedBlock ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
						if( pSecondJob->pattern == JP_AIAI || pSecondJob->pattern == JP_ANAI ){
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else{
							partiallyAvailable.push_back( ALVStatus->GetID() );
						}	
					}
					else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TravelToDesignatedQC ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
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
					if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TransferAtBlock ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
						fullyAvailable.push_back( ALVStatus->GetID() );
					}
					else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TransferAtQC ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
						fullyAvailable.push_back( ALVStatus->GetID() );
					}
				}
				else if( nJobCount == 1 ){ //현재 수행 중인 작업이 하나 뿐인 ALV
					if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TransferAtBlock ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
						fullyAvailable.push_back( ALVStatus->GetID() );
					}
					else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TransferAtQC ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
						fullyAvailable.push_back( ALVStatus->GetID() );
					}
					else{//작업 수행이 끝나가는 중이 아니더라도 20ft 작업 하나 처리 중이면 partial Available ALV이다.
						partiallyAvailable.push_back( ALVStatus->GetID() );
					}					
				}
				else if( nJobCount == 2 ){	//20ft 작업을 두 개 들고 있는 경우이다.
					CALVJob* pSecondJob = ALVStatus->GetSecondJob();
					if( pCurrentJob->prsType == PrsType_Discharging && pCurrentJob->jobState == JobState_TransferAtBlock ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
						if( pSecondJob->pattern == JP_AIAI || pSecondJob->pattern == JP_ANAI ){
							fullyAvailable.push_back( ALVStatus->GetID() );
						}
						else{
							partiallyAvailable.push_back( ALVStatus->GetID() );
						}						
					}
					else if( pCurrentJob->prsType == PrsType_Loading && pCurrentJob->jobState == JobState_TransferAtQC ){//작업 수행이 끝나가는 시점이면 계획 대상에 포함시킨다.
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
	//작업 할당
	pALVJob->assignedEquipID = equipID;//작업에 ALV 연결
	ALVInfo[equipID]->AddJob(pALVJob);//ALV에 작업 연결

	//목록 갱신
	assignedJob.insert(make_pair(pALVJob->jobID, pALVJob ));
	unassignedJob.erase(pALVJob->jobID);

	//Emulator에 알림
	m_pALVManager->AssignJob( equipID, pALVJob );

	return;
}


//private operation for single-carrier mode --------------------------------------------------------------------------
void CALVScheduler::UsingEarliestDeadlineRule(map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable )
{
	deque<pair<int, string>> orderedList; 
	SortUnassignedJob(orderedList, unassignedJob );//unassignedJob을 deadlne이 빠른 순으로 정렬한다.

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
		CALVJob* pALVJob = FindMinimumInventoryJob(unassignedJob);//인벤토리 레벨이 가장 작은 것을 찾는다.
		AssignJobToALV( *indexALV, pALVJob, ALVInfo, assignedJob, unassignedJob );
		m_InventoryLevel[pALVJob->QCID] += 1;
	}
	return;
}

CALVJob* CALVScheduler::FindMinimumInventoryJob( map<string,CALVJob*>& unassignedJob )
{
	//임의의 최소값 할당하기
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

	//최소값 찾기
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
				if( rand()%100 > 50 ){//인벤토리 레벨과 데드라인이 동일한 경우 렌덤 셀렉션
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
	//Step A. unassignedJob을 deadlne이 빠른 순으로 정렬한다.
	deque<pair<int, string>> orderedList; 
	SortUnassignedJob(orderedList, unassignedJob );


	//Step B. Local Search를 위한 초기해 생성
	int nAvailableALV = 2*(int)fullyAvailable.size() + (int)partiallyAvailable.size();
	vector<string> solution[POPULATION_SIZE];
	deque<string> idList;
	idList.insert(idList.begin(), fullyAvailable.begin(), fullyAvailable.end() );
	idList.insert(idList.end(), partiallyAvailable.begin(), partiallyAvailable.end() );
	GenerateInitialSolution( solution, idList, nAvailableALV, orderedList, unassignedJob, fullyAvailable, partiallyAvailable );

	//Step C. 초기해 평가
	//...기본값 생성
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
	//...초기해 중 가장 좋은 것 선택
	vector<string> bestSoFar;
	bestSoFar.insert(bestSoFar.begin(), solution[minSolution].begin(), solution[minSolution].end() );

	/*
	//Step D. 반복 탐색
	int previousCost = minimumCost;
	while( previousCost < minimumCost ){//재탐색 결과 더 좋은 해를 찾지 못하였다면 탐색 종료
		GenerateNeighbor( solution, bestSoFar, idList, nAvailableALV );//이웃해 생성

		for( i=0; i<10; ++i){//이웃해 평가
			//DecodingSolution( solution[i], orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
			cost = EvaluateSolution( solution[i], orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
			if( cost < minimumCost ){
				minimumCost = cost;
				minSolution = i;
			}
		}
		if( previousCost > minimumCost ){//새로 찾은 해가 더 좋다면
			bestSoFar.clear();
			bestSoFar.insert(bestSoFar.begin(), solution[minSolution].begin(), solution[minSolution].end() );
			previousCost = minimumCost;
		}
		else{//탐색 종료
			break;
		}
	}
	*/

	//Step E. 20ft 이어진 작업 처리 순서 결정
	DecideDualLoadingSequence( bestSoFar, orderedList, ALVInfo, fullyAvailable, partiallyAvailable, assignedJob, unassignedJob );
	

	//Step F. 작업 할당
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

	map<string, bool> checkList;//작업 처리 순서 결정 여부를 체크
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
				checkList[orderedList[i].second] = true;//순서 결정 여부 갱신
				pFirstJob = unassignedJob[orderedList[i].second];
				if( pFirstJob->container.conType == ConType_General20ft ){//20ft인 경우
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
	//Full인 경우 20ft 두 개 또는 40ft 하나 할당 가능
	//Partial인 경우 어떤 작업이든 하나만 할당 가능
	int i,j;
	map<string, int> assignment;
	for( i=0; i<(int)idList.size(); ++i){
		assignment.insert(make_pair(idList[i],0));
	}
	
	int solutionSize = (int)orderedList.size();//해의 길이

	deque<string> checkList;
	deque<string>::iterator where;
	map<string,int>::iterator init;
	
	for(i = 0; i < POPULATION_SIZE; ++i ){
		//변수 초기화
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

			//해당 위치의 작업을 가져온다.
			CALVJob* pALVJob = unassignedJob[orderedList[j].second];
			if( pALVJob->container.conType == ConType_General20ft ){
				assignment[equipID] += 1;
			}
			else{
				assignment[equipID] += 2;
			}

			//할당한 ALV가 Full인지 Partial인지
			where = find( fullyAvailable.begin(), fullyAvailable.end(), equipID	);
			if( where != fullyAvailable.end() ){//Full A
				if( assignment[equipID] >= 2 ){//더 이상 할당 불가
					where = find( checkList.begin(), checkList.end(), equipID	);
					checkList.erase(where);
				}
			}
			else{//Partial A
				if( assignment[equipID] > 0 ){//더 이상 할당 불가
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
	//일단은 Mutation만 구현하여 동작 확인
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

	map<string, bool> checkList;//작업의 cost 계산 여부를 체크
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
			if( checkList[orderedList[i].second] == false ){//cost 계산
				checkList[orderedList[i].second] = true;
				pFirstJob = unassignedJob[orderedList[i].second];
				if( pFirstJob->container.conType == ConType_General40ft ){//40ft인 경우
					long currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
					if( currentTime > pFirstJob->deadline ){//이미 수행하여야할 시간이 지났다면
						totalCost -= currentTime - pFirstJob->deadline;//가중치
					}
					else{
						finishTime = currentTime + m_pALVManager->GetTravelTimefromMap(ALVInfo[equipID]->m_WPID, pFirstJob->QCID );
						if( finishTime > pFirstJob->deadline ){
							totalCost += finishTime - pFirstJob->deadline;							 
						}	
					}
				}
				else{//20ft 작업인 경우 - 하나 더 할당되었는지 살필 것.
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
								totalCost -= currentTime - pFirstJob->deadline;//가중치
							}
							else{
								totalCost -= currentTime - pSecondJob->deadline;//가중치
							}
						}
						else{
							totalCost += cost;
						}
					}
					else{
						long currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
						if( currentTime > pFirstJob->deadline ){//이미 수행하여야할 시간이 지났다면
							totalCost -= currentTime - pFirstJob->deadline;//가중치
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
		else{//Partial의 경우
			if( checkList[orderedList[i].second] == false ){//cost 계산
				checkList[orderedList[i].second] = true;
				pFirstJob = ALVInfo[equipID]->GetCurrentJob();		//이전에 할당된 작업을 가져온다.
				pSecondJob = unassignedJob[orderedList[i].second];	//현재 할당할 작업을 가져온다.
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
	//할당된 Fully A에 대하여
	for( index = fullyAvailable.begin(); index != fullyAvailable.end(); ++index ){
		where = find(solution.begin(), solution.end(), (*index) );
		if( where != solution.end() ){//현재 ALV가 solution에 존재한다면
			where = find(checkList.begin(), checkList.end(), (*index) );
			checkList.erase( where );//일단 checkList에서 지운다.

			//Fully A는 20ft 2개 또는 40ft 하나만 할당 가능하다.
			int i, nCapacity = 0;
			for( i = 0; i<(int)solution.size(); ++i){
				if( solution[i] == (*index) ){
					if( nCapacity >= 2 ){//할당량 채워진 상태이면
						if( checkList.empty() ){
							solution[i] = "";
						}
						else{
							solution[i] = checkList[rand()%(int)checkList.size()];
						}
					}
					else{
						//해당 위치의 작업을 가져온다.
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
	//할당된 Partially A에 대하여
	for( index = partiallyAvailable.begin(); index != partiallyAvailable.end(); ++index ){
		where = find(solution.begin(), solution.end(), (*index) );
		if( where != solution.end() ){//현재 ALV가 solution에 존재한다면
			where = find(checkList.begin(), checkList.end(), (*index) );
			checkList.erase( where );//일단 checkList에서 지운다.

			//Partially A는 20ft 하나 또는 40ft 하나만 할당 가능하다.
			int i, nJob = 0;
			for( i = 0; i<(int)solution.size(); ++i){
				if( solution[i] == (*index) ){
					if( nJob >= 1 ){//두번째 작업부터 ALV 교체
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

	//초기 계획에 포함되어있지 않던 ALV에 대하여 검사
	vector<string>::iterator cindex;
	deque<string>::iterator cwhere;
	for( cindex = checkList.begin(); cindex != checkList.end(); ++cindex ){
		where = find(solution.begin(), solution.end(), (*cindex) );
		if( where != solution.end() ){//현재 ALV가 solution에 존재한다면
			//먼저 Full Partial 확인
			cwhere = find(fullyAvailable.begin(), fullyAvailable.end(), (*cindex) );
			if( cwhere != fullyAvailable.end() ){//Full이면
				//Fully A는 20ft 2개 또는 40ft 하나만 할당 가능하다.
				int i, nCapacity = 0;
				for( i = 0; i<(int)solution.size(); ++i){
					if( solution[i] == (*cindex) ){
						if( nCapacity >= 2 ){//할당량 채워진 상태이면
							solution[i] = "";
						}
						else{
							//해당 위치의 작업을 가져온다.
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
			else{//Partial이면
				int i, nJob = 0;
				for( i = 0; i<(int)solution.size(); ++i){
					if( solution[i] == (*cindex) ){
						if( nJob >= 1 ){//두번째 작업부터 ALV 교체
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
	SortUnassignedJob(orderedList, unassignedJob );//unassignedJob을 deadlne이 빠른 순으로 정렬한다.

	deque<pair<int, string>>::iterator indexList;
	deque<string>::iterator where;
	while( orderedList.empty() == false ){
		indexList = orderedList.begin();
		
		if( fullyAvailable.empty() && partiallyAvailable.empty() ){
			break;
		}

		//해당 작업에 대하여 ALV가 작업을 수행 가능한 시점을 계산한다.]
		CALVJob* pNewALVJob = unassignedJob[indexList->second];
		if( pNewALVJob->prsType == PrsType_Loading ){
			//적하 작업인 경우/QC의 계획 상 순서가 앞인 적하 작업에 차량이 할당되지 않았다면 PASS
			if( m_pALVManager->CheckKeepingSchedule(pNewALVJob) == false ){
				orderedList.erase(indexList);
				continue;
			}
		}
		/*
		//아래와 같이 처리할 경우 40ft 작업을 거의 처리 안함
		if( pNewALVJob->container.conType == ConType_General40ft && fullyAvailable.empty() == true ){//40ft작업 수행 불가 처리
			orderedList.erase(indexList);
			continue;
		}
		*/

		deque<pair<int, string>> sortedALVList;
		SortALVList(sortedALVList, pNewALVJob, ALVInfo, fullyAvailable, partiallyAvailable);//40ft작업인 경우 이 함수 안에서 partial 배제

		
		deque<pair<int, string>>::iterator indexALV= sortedALVList.begin();
		//Fully이고 20ft 작업인 경우에는 Best 선택
		where = find( fullyAvailable.begin(), fullyAvailable.end(), indexALV->second );
		if( where != fullyAvailable.end() && pNewALVJob->container.conType == ConType_General20ft ){
			AssignJobToALV( indexALV->second, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
		}
		else{
			//40ft 작업이거나 Partially인 경우에는 Hurdle 선택
			if( indexALV->first >= pNewALVJob->deadline ){//첫번째 ALV가 이미 데드라인 내에 수행 불가라면 바로 선택
				AssignJobToALV( indexALV->second, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
			}
			else{
				bool assigned = false;
				for( ; indexALV != sortedALVList.end(); ++indexALV ){
					if( indexALV->first >= pNewALVJob->deadline ){//데드라인 통과 못한다면 이전 작업을 할당
						--indexALV;
						AssignJobToALV( indexALV->second, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
						assigned = true;
						break;
					}
				}
				if( assigned == false ){//모든 차량이 데드라인 통과 가능한 경우이다.
					--indexALV;
					AssignJobToALV( indexALV->second, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
				}
			}
		}
		//작업 삭제
		orderedList.erase(indexList);
		//ALV 삭제
		where = find( fullyAvailable.begin(), fullyAvailable.end(), indexALV->second );
		if( where != fullyAvailable.end() ){
			string equipID = (*where);
			fullyAvailable.erase(where);

			if( pNewALVJob->container.conType == ConType_General20ft ){//fullyAvailable에 20ft 작업을 할당한 경우
				//vehicle-initiated Rule로 20ft 작업 하나 더 할당
				CALVJob* pAssignedJob = pNewALVJob;//계획 중에 할당된 작업이 AssignedJob
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
					else{//40ft작업 수행 불가
						continue;
					}
				}
				if( shortestJob != orderedList.end() ){//작업 할당된 경우
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
			CALVJob* pAssignedJob;//이전에 할당된 작업을 가져온다.
			if( ALVInfo[*index]->GetCountofJob() >= 2 ){//계획 중에 할당된 작업이 있는 경우
				pAssignedJob = ALVInfo[*index]->GetSecondJob();
			}
			else{//계획 전에 할당된 작업만 있는 경우
				pAssignedJob = ALVInfo[*index]->GetCurrentJob();//이전에 할당된 작업을 찾는다.
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


	while( partiallyAvailable.empty() == false ){//20ft 작업 할당
		if( unassignedJob.empty() == true ){
			break;
		}
		indexALV = partiallyAvailable.begin();
		CALVJob* pAssignedJob;//이전에 할당된 작업을 찾는다.
		if( ALVInfo[*indexALV]->GetCountofJob() >= 2 ){//계획 중에 할당된 작업이 있는 경우
			pAssignedJob = ALVInfo[*indexALV]->GetSecondJob();
		}
		else{//계획 전에 할당된 작업만 있는 경우
			pAssignedJob = ALVInfo[*indexALV]->GetCurrentJob();//이전에 할당된 작업을 찾는다.
		}
		//travelTime = m_pALVManager->GetTravelTimeEstimated(*indexALV, pAssignedJob );//어차피 공통. 따라서 계산할 필요 없음

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
		else{//partially Available이 수행 가능한 작업이 없음
			break;
		}
	}
	
	
	return;
}



void CALVScheduler::UsingDualShortestTravelTime(map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob )
{
	deque<pair<int, string>> orderedList; 
	SortUnassignedJob(orderedList, unassignedJob );//unassignedJob을 deadlne이 빠른 순으로 정렬한다.

	//각 작업에 대하여 shorteset travel time을 가지는 ALV를 할당한다.
	deque<string>::iterator partialIndex, fullIndex, shortestIndex;
	int travelTime, shortestTravelTime;

	deque<pair<int, string>>::iterator index;//deque<(deadline,JobID)> 형태
	for(index = orderedList.begin(); index != orderedList.end(); ++index ){
		if( fullyAvailable.empty() && partiallyAvailable.empty() ){
			break;
		}

		CALVJob* pNewALVJob = unassignedJob[index->second];

		//초기해를 정한다.
		bool partialALV = false;//할당된 ALV가 어떤 목록의 ALV인지 체크
		if( fullyAvailable.empty() == false ){
			shortestIndex = fullyAvailable.begin();
			shortestTravelTime = m_pALVManager->GetTravelTimefromMap(ALVInfo[*shortestIndex]->m_WPID, pNewALVJob->QCID );
		}
		else{
			if( pNewALVJob->container.conType != ConType_General20ft ){
				continue;//40ft 작업을 수행할 수 있는 ALV가 없는 경우
			}
			shortestIndex = partiallyAvailable.begin();
			CALVJob* pAssignedJob;//이전에 할당된 작업을 찾는다.
			if( ALVInfo[*shortestIndex]->GetCountofJob() >= 2 ){//계획 중에 할당된 작업이 있는 경우
				pAssignedJob = ALVInfo[*shortestIndex]->GetSecondJob();
			}
			else{//계획 전에 할당된 작업만 있는 경우
				pAssignedJob = ALVInfo[*shortestIndex]->GetCurrentJob();//이전에 할당된 작업을 찾는다.
			}			
			shortestTravelTime = m_pALVManager->GetTravelTimeEstimated(*shortestIndex, pAssignedJob);
			shortestTravelTime += m_pALVManager->GetTravelTimefromMap(pAssignedJob->QCID, pNewALVJob->QCID);
			partialALV = true;
		}

		if( fullyAvailable.empty() == false ){//먼저 fully-available ALV 중에서 STT를 가지는 것을 찾는다.
			for( fullIndex = fullyAvailable.begin(); fullIndex != fullyAvailable.end(); ++fullIndex ){
				travelTime = m_pALVManager->GetTravelTimefromMap(ALVInfo[*fullIndex]->m_WPID, pNewALVJob->QCID );
				if( shortestTravelTime > travelTime ){
					shortestTravelTime = travelTime;
					shortestIndex = fullIndex;
					partialALV = false;
				}
			}
		}		
		if( partiallyAvailable.empty() == false && pNewALVJob->container.conType == ConType_General20ft){//탐색 대상을 partially available한 ALV로 확장한다.
			for( partialIndex = partiallyAvailable.begin(); partialIndex != partiallyAvailable.end(); ++partialIndex ){
				CALVJob* pAssignedJob;//이전에 할당된 작업을 찾는다.
				if( ALVInfo[*partialIndex]->GetCountofJob() >= 2 ){//계획 중에 할당된 작업이 있는 경우
					pAssignedJob = ALVInfo[*partialIndex]->GetSecondJob();
				}
				else{//계획 전에 할당된 작업만 있는 경우
					pAssignedJob = ALVInfo[*partialIndex]->GetCurrentJob();//이전에 할당된 작업을 찾는다.
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

		if( partialALV ){//이 중에서 할당되었다면
			pNewALVJob->pattern = JP_ANAN;
			AssignJobToALV( *shortestIndex, pNewALVJob, ALVInfo, assignedJob, unassignedJob );
			partiallyAvailable.erase(shortestIndex);
		}
		else{//full ALV에서 할당되었다면
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
		//20x20 작업만 Dual-loading
		CALVJob* pALVJob = FindMinimumInventoryJob(unassignedJob);//인벤토리 레벨이 가장 작은 것을 찾는다.
		AssignJobToALV( *indexALV, pALVJob, ALVInfo, assignedJob, unassignedJob );
		if( pALVJob->container.conType == ConType_General20ft ){
			if( pALVJob->connectedJobList.empty() == false ){//20x20 작업인 경우
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
				if( index != pALVJob->connectedJobList.end() ){//해당 작업이 unassignedJob 목록에 있는지 확인한다.
					for( indexJob = unassignedJob.begin(); indexJob != unassignedJob.end(); ++indexJob ){
						if( indexJob->second->jobID == index->second ){//작업 할당
							//작업 타입 결정
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

	map<string, CALVJob*> assignedJob;//계획 중 할당한 작업에 대하여 dual-load를 지원하기 위해

	while( unassignedJob.empty() == false ){//미할당된 작업이 없을 때까지 반복
		if(  fullyAvailable.empty() ){//가용 ALV가 없을 때까지 반복
			if( !partiallyAvailable.empty() ){//20ft 작업 수행만 가능한 ALV
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
		

		//Step.A. 미할당된 작업 중 earliest deadline을 찾는다.
		int earliestDeadline = -1;
		map<string,CALVJob*>::iterator earliestDeadlineJob = unassignedJob.begin();
		for( indexJob = unassignedJob.begin(); indexJob != unassignedJob.end(); ++indexJob ){
			if( earliestDeadline == -1 && indexJob->second->deadline > 0  ){//초기화
				earliestDeadline = indexJob->second->deadline;
				earliestDeadlineJob = indexJob;
			}
			else if( earliestDeadline > indexJob->second->deadline ){//데드라인 비교
				earliestDeadline = indexJob->second->deadline;
				earliestDeadlineJob = indexJob;
			}
			else if( earliestDeadline == indexJob->second->deadline ){//deadline이 동일하다면
				if( earliestDeadlineJob->second->prsType == PrsType_Loading && indexJob->second->prsType == PrsType_Loading ){//모두 적하이면
					if( GETPROCESSID(earliestDeadlineJob->second->jobID) > GETPROCESSID(indexJob->second->jobID) ){//순서가 빠른 것을 먼저
						earliestDeadline = indexJob->second->deadline;
						earliestDeadlineJob = indexJob;
					}
				}
				else{
					if( earliestDeadlineJob->second->prsType != PrsType_Loading && indexJob->second->prsType == PrsType_Loading ){//적하를 우선
						earliestDeadline = indexJob->second->deadline;
						earliestDeadlineJob = indexJob;
					}
				}
			}
		}

		//Step.B. ALV에 작업 할당 시 예상되는 QC 지연을 측정한다.
		pair<int,int> lowestCost;
		deque<string>::iterator lowestCostALV;
		EJobPattern JobPatternDetermined = JP_Undefined;

		//초기값 정하기
		if( fullyAvailable.empty() == false ){
			lowestCostALV = fullyAvailable.begin();			
			lowestCost = EstimateCost( ALVInfo[*lowestCostALV], earliestDeadlineJob->second );
		}
		else{
			lowestCostALV = partiallyAvailable.begin();

			CALVJob* pCurrentJob;			

			//계획 중에 할당된 작업 찾기
			map<string, CALVJob*>::iterator index;
			for( index = assignedJob.begin(); index != assignedJob.end(); ++index ){
				if( *lowestCostALV == index->first ){
					pCurrentJob = index->second;
					break;
				}
			}
			//계획 전에 할당되었던 작업인 경우
			if( index == assignedJob.end() ){	
				pCurrentJob = ALVInfo[*lowestCostALV]->GetCurrentJob();
			}

			EJobPattern JobPattern = JP_Undefined;
			lowestCost = EstimateCostforDualLoad( ALVInfo[*lowestCostALV], pCurrentJob, earliestDeadlineJob->second, JobPattern );
			JobPatternDetermined = JobPattern;
		}

		//최소값 찾기
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
			if( earliestDeadlineJob->second->container.conType == ConType_General20ft ){//이 경우 Partially Available도 고려
				for( partialALV = partiallyAvailable.begin(); partialALV != partiallyAvailable.end(); ++ partialALV ){
					//할당된 작업 찾기
					CALVJob* pCurrentJob;			
					map<string, CALVJob*>::iterator index;
					for( index = assignedJob.begin(); index != assignedJob.end(); ++index ){
						if( *partialALV == index->first ){
							pCurrentJob = index->second;
							break;
						}
					}
					if( index == assignedJob.end() ){	//계획 전에 할당되었던 작업인 경우
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


		//Step.C. Lowest assignment cost를 가지는 것을 선택, 
		//m_Schedule.push_back( make_pair(*lowestCostALV, earliestDeadlineJob->first) );	//ALVID JobID
		assignedJob.insert( make_pair( *lowestCostALV, earliestDeadlineJob->second) );

		//Available ALV 목록 갱신
		if( earliestDeadlineJob->second->container.conType == ConType_General20ft ){
			emptyALV = find( fullyAvailable.begin(), fullyAvailable.end(), *lowestCostALV);
			if( emptyALV != fullyAvailable.end() ){	// Empty ALV에 20ft 작업을 할당했다면 --> partially available 목록에 추가 후, fully에서 삭제
				partiallyAvailable.push_back(*lowestCostALV);
				fullyAvailable.erase( emptyALV );
			}
			else{
				partialALV = find( partiallyAvailable.begin(), partiallyAvailable.end(), *lowestCostALV);
				if( partialALV != partiallyAvailable.end() ){
					partiallyAvailable.erase( partialALV );

					//20x20ft 작업이 된 경우이므로 작업 수행 패턴을 지정해줌
					earliestDeadlineJob->second->pattern = JobPatternDetermined;
				}
			}
		}
		else{//Empty에 40ft 할당한 경우라면
			emptyALV = find( fullyAvailable.begin(), fullyAvailable.end(), *lowestCostALV);
			if( emptyALV != fullyAvailable.end() ){
				fullyAvailable.erase( emptyALV );
			}
		}
		unassignedJob.erase(earliestDeadlineJob);										//unassigned order 갱신


	}

	//InformSchedule();
	return;
}

pair<int,int> CALVScheduler::EstimateCost( CALVInfo* pALVInfo, CALVJob* pALVJob ){
	int cost = 0;
	int travelTime = 0;
	int estimatedFinishTime;
	if( pALVJob->prsType == PrsType_Discharging	 ){//양하인 경우 QC까지 가는 것만 계산
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pALVJob->QCID + "Q0" );		
	}
	else if( pALVJob->prsType == PrsType_Loading ){//적하인 경우 Block으로 가는 시간 + QC로 가는 시간 계산
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pALVJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pALVJob->BlockID + "S0", pALVJob->QCID + "Q0" );		
	}

	estimatedFinishTime = travelTime + (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);//현재 시각 + 주행에 필요한 시간
	if( estimatedFinishTime < pALVJob->deadline ){
		cost = 0; //QC 지연이 발생하지 않았다.				
	}
	else{
		cost = estimatedFinishTime - pALVJob->deadline;
	}

	return make_pair(cost, travelTime);
}

pair<int,int> CALVScheduler::EstimateCostforDualLoad( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob, EJobPattern& JobPattern )
{
	//AANN, ANAN, ANNA 각각을 계산하여 cost가 낮은 것을 리턴	
	pair<int, int> lowestCost;	

	//ANN만 가능한 경우
	if( pAssignedJob->prsType == PrsType_Discharging && ( pAssignedJob->jobState == JobState_TravelToYardside || pAssignedJob->jobState == JobState_TravelToDesignatedBlock) ){
		//양하 작업의 Drop-off를 수행하러 가는 중
		lowestCost = GetANNcost( pALVInfo, pAssignedJob, pNewJob );
		JobPattern = JP_AANN;
		return lowestCost;

	}
	else if( pAssignedJob->prsType == PrsType_Loading && (pAssignedJob->jobState == JobState_TravelToQuayside || pAssignedJob->jobState == JobState_TravelToDesignatedQC) ){
		//적하 작업의 Drop-off를 수행하러 가는 중
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
		//AI인 경우
		if( pAssignedJob->prsType == PrsType_Discharging && pAssignedJob->CheckConnectedJob(pNewJob->jobID) ){
			//AIAI, AINA, AIAN 가능
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
			//ANAI 가능
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
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
	}
	else{// if( pNewJob->prsType ==PrsType_Loading ){	
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->QCID + "Q0" );
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
	}	
	
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pNewJob->deadline ){//이미 데드라인을 지난 경우
		cost = 0;
	}
	else{
		int estimatedFinishTime = currentTime + travelTime;
		if( estimatedFinishTime < pNewJob->deadline ){
			cost = 0;//QC 지연이 발생하지 않았다.				
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
		travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		//NN
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
	}
	else{// if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pAssignedJob->QCID + "Q0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		//NN
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
	}
	
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pNewJob->deadline ){//이미 데드라인을 지난 경우
		cost = 0;
	}
	else{
		int estimatedFinishTime = currentTime + travelTime;
		if( estimatedFinishTime < pNewJob->deadline ){
			cost = 0;//QC 지연이 발생하지 않았다.				
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
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pAssignedJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap(pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			assignedTravelTime = travelTime + m_pALVManager->GetTravelTimefromMap( pNewJob->QCID + "Q0", pAssignedJob->QCID + "Q0" );
			assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pAssignedJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			assignedTravelTime = travelTime;
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
	}
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//이미 데드라인을 지난 경우
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC 지연이 발생하지 않았다.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC 지연이 발생하지 않았다.				
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
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		if( pNewJob->prsType ==PrsType_Discharging ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->QCID + "Q0");
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			assignedTravelTime = travelTime + m_pALVManager->GetTravelTimefromMap( pNewJob->QCID + "Q0", pNewJob->BlockID + "S0" );
			assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			assignedTravelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pAssignedJob->QCID + "Q0" );
			assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
		else if( pNewJob->prsType ==PrsType_Loading ){
			travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
			travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
			assignedTravelTime = travelTime + m_pALVManager->GetTravelTimefromMap( pNewJob->QCID + "Q0", pAssignedJob->QCID + "Q0" );
			assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		}
	}
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//이미 데드라인을 지난 경우
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC 지연이 발생하지 않았다.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC 지연이 발생하지 않았다.				
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
	if( currentTime > pNewJob->deadline ){//이미 데드라인을 지난 경우
		cost = 0;
	}
	else{
		int estimatedFinishTime = currentTime + travelTime;
		if( estimatedFinishTime < pNewJob->deadline ){
			cost = 0;//QC 지연이 발생하지 않았다.				
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
		travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		assignedTravelTime = travelTime;
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->QCID + "Q0", pNewJob->QCID + "Q0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
	}

	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//이미 데드라인을 지난 경우
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC 지연이 발생하지 않았다.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC 지연이 발생하지 않았다.				
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
		travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		assignedTravelTime = travelTime + m_pALVManager->GetTravelTimefromMap( pNewJob->QCID + "Q0", pAssignedJob->QCID + "Q0" );
		assignedTravelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
	}

	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//이미 데드라인을 지난 경우
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC 지연이 발생하지 않았다.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC 지연이 발생하지 않았다.				
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
		travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
	}
	else if( pAssignedJob->prsType == PrsType_Loading ){
		travelTime = m_pALVManager->GetTravelTimefromMap( pALVInfo->m_WPID, pAssignedJob->BlockID + "S0" );
		travelTime += m_pALVManager->GetTravelTimefromMap( pAssignedJob->BlockID + "S0", pNewJob->BlockID + "S0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		travelTime += m_pALVManager->GetTravelTimefromMap( pNewJob->BlockID + "S0", pNewJob->QCID + "Q0" );
		travelTime += 50 + g_SimulationSpec.nVehicle;//평균 HP 대기 시간 (새로 계산해볼 것)
		assignedTravelTime = travelTime;
	}
	int currentTime = (int)(m_pALVManager->GetClockTick() * g_SimulationSpec.unitTime);
	if( currentTime > pAssignedJob->deadline || currentTime > pNewJob->deadline ){//이미 데드라인을 지난 경우
		cost = 0;
	}
	else{
		int assignedFinishTime = assignedTravelTime + currentTime;
		int	estimatedFinishTime = travelTime + currentTime;

		if( assignedFinishTime < pAssignedJob->deadline ){
			cost = 0; //QC 지연이 발생하지 않았다.				
		}
		else{
			cost = assignedFinishTime - pAssignedJob->deadline;
		}

		if( estimatedFinishTime < pNewJob->deadline ){
			cost += 0; //QC 지연이 발생하지 않았다.				
		}
		else{
			cost += estimatedFinishTime - pNewJob->deadline;
		}
	}
	return make_pair(cost, travelTime);
}


