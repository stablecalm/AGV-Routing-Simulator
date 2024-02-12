#pragma once

using namespace std;

enum EPrsType {
	PrsType_Undefined,		// 정의되지 않음(?)
	PrsType_Loading,			// 적하
	PrsType_Discharging,		// 양하
	PrsType_CarryIn,		// 반입	
	PrsType_CarryOut,		// 반출
	PrsType_Rehandle		// 재취급, 재정돈 등
};

enum EEquipType {			// 장비 종류
	EquipType_Undefined,
	EquipType_QC,
	EquipType_AGV,
	EquipType_ALV,
	EquipType_ATC,
	EquipType_ET
};

enum ECargoType{			//물량 타입
	CargoType_Undefined,
	CargoType_Discharge,		//양하만 있는 경우
	CargoType_Load,				//적하만 있는 경우
	CargoType_Mixed,			//양하 + 적하
	CargoType_DoubleCycle		//더블 사이클 포함
};

enum EDispatchRule{
	DR_MinimumInventory,
	DR_EarliestDeadline,
	DR_NearestVehicle,
	DR_LongestIdle,
	DR_GA,
	//for dual-load
	DR_GRASP,
	DR_HurdleJump,			//Proposed Heuristic
	DR_DualSTT,				//Shortest Travel Time(workcenter-initiated)
	DR_DualSTT2,			//Shortest Travel Time(vehicle-initiated)
	DR_DualMIL,				//Minimum Inventory Level
	DR_DualPatternBased
	
};