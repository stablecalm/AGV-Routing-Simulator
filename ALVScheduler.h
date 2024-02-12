#pragma once
#include "ALVManager.h"
#include "Process.h"

using namespace std;

class CALVManager;

class CALVScheduler
{
private:
	CALVManager*	m_pALVManager;
	EDispatchRule	m_DispatchRule;
	map<string, int>m_InventoryLevel;

private:
	//내부 연산은 참조로 처리
	void	GetAvailableALV( map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable, deque<string>& partiallyAvailable );
	void	SortUnassignedJob( deque<pair<int, string>>& orderedList, map<string, CALVJob*>& unassignedJob );
	void	AssignJobToALV(string equipID, CALVJob* pALVJob, map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob );

	CALVJob*FindMinimumInventoryJob( map<string,CALVJob*>& unassignedJob );

	//Single-carrier mode 계획 함수	
	void	UsingEarliestDeadlineRule(map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable);
	void	UsingMinimumInventoryRule(map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable);

	//Dual-carrier mode 계획 함수
	void	UsingDualHurdleJump( map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob );
	void	SortALVList( deque<pair<int, string>>& sortedALVList, CALVJob* pALVJob, map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable);

	//ver. GRASP
	void	UsingDualGRASP( map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob );
	void	GenerateInitialSolution( vector<string>* solution, deque<string>& idList, int nAvailableALV, deque<pair<int, string>>& orderedList, map<string, CALVJob*>& unassignedJob,deque<string>& fullyAvailable, deque<string>& partiallyAvailable );
	void	GenerateNeighbor( vector<string>* solution, vector<string>& bestSoFar, deque<string>& idList, int nAvailableALV  );
	void	DecodingSolution( vector<string>& solution, deque<pair<int, string>>& orderedList, map<string,CALVInfo*>& ALVInfo, 
			deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob );
	int		EvaluateSolution( vector<string>& solution, deque<pair<int, string>>& orderedList, map<string,CALVInfo*>& ALVInfo, 
			deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob );
	void	DecideDualLoadingSequence( vector<string>& bestSoFar, deque<pair<int, string>>& orderedList, map<string,CALVInfo*>& ALVInfo, 
			deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob );

	//workcenter-initiated
	void	UsingDualShortestTravelTime( map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob );
	//vehicle-initiated
	void	UsingDualShortestTravelTime2( map<string,CALVInfo*>& ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable,map<string,CALVJob*>& assignedJob,  map<string, CALVJob*>& unassignedJob );

	void	UsingDualInventoryRule(map<string,CALVInfo*>& ALVInfo, map<string,CALVJob*>& assignedJob, map<string,CALVJob*>& unassignedJob, deque<string>& fullyAvailable);
	void	UsingPatternBasedHeuristic( map<string,CALVInfo*> ALVInfo, deque<string>& fullyAvailable, deque<string>& partiallyAvailable, map<string, CALVJob*> unassignedJob );
	pair<int,int> EstimateCost( CALVInfo* pALVInfo, CALVJob* pALVJob );
	pair<int,int> EstimateCostforDualLoad( CALVInfo* pALVInfo, CALVJob* pCurrentJob, CALVJob* pALVJob, EJobPattern& JobPattern );
	pair<int,int> GetANNcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob );
	pair<int,int> GetAANNcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob );
	pair<int,int> GetANANcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob );
	pair<int,int> GetANNAcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob );
	pair<int,int> GetAIAIcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob );
	pair<int,int> GetAIANcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob );
	pair<int,int> GetAINAcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob );
	pair<int,int> GetANAIcost( CALVInfo* pALVInfo, CALVJob* pAssignedJob, CALVJob* pNewJob );

public:
	CALVScheduler(CALVManager* pALVManager);
	~CALVScheduler(void);

	void	MakeSchedule(map<string, CALVInfo*>& ALVInfo, map<string, CALVJob*>& assignedJob, map<string, CALVJob*>& unassignedJob );
	void	InformJobComplete( CALVJob* pALVJob );

	
};
