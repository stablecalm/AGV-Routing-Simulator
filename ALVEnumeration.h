#pragma once

using namespace std;

enum EPrsType {
	PrsType_Undefined,		// ���ǵ��� ����(?)
	PrsType_Loading,			// ����
	PrsType_Discharging,		// ����
	PrsType_CarryIn,		// ����	
	PrsType_CarryOut,		// ����
	PrsType_Rehandle		// �����, ������ ��
};

enum EEquipType {			// ��� ����
	EquipType_Undefined,
	EquipType_QC,
	EquipType_AGV,
	EquipType_ALV,
	EquipType_ATC,
	EquipType_ET
};

enum ECargoType{			//���� Ÿ��
	CargoType_Undefined,
	CargoType_Discharge,		//���ϸ� �ִ� ���
	CargoType_Load,				//���ϸ� �ִ� ���
	CargoType_Mixed,			//���� + ����
	CargoType_DoubleCycle		//���� ����Ŭ ����
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