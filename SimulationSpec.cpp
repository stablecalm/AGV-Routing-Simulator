#include "StdAfx.h"
#include "SimulationSpec.h"

CSimulationSpec::CSimulationSpec(void)
{
	//...simulation setting
	showGraphicInterface = true;
	autostart = false;	
	cargoType	= CargoType_Discharge;

	//...vehicle settings
	dispType	= DR_MinimumInventory;
	dualLoad	= false;

	//...QC setting
	doubleCycling = false;			//QC의 double-cycling 지원 여부
	QCHPGap = 700;

	halfCycletime = 60;		//시간 당 30 box 처리한다고 가정 - Single cycle은 120초
	avgWaitingTime = 0;

	blockGap = 1200;
	blockHPGap = 700;

	//HP Information
	//...HP size;
	hpWidth = 510;
	hpHalfWidth = 255;
	hpLength = 1300;
	hpHalfLength = 650;
	i20ftHalfGap = 50;			//HP에 놓이는 20피트 컨테이너 간 갭


	ifstream ifs;
	//...simulation parameters		
	ifs.open( "SPEC_Simulation.txt" );
	if(	!ifs.fail() ){
		string variableDescription;
		ifs >> variableDescription;
		ifs >> unitTime;
		ifs >> variableDescription;
		ifs >> delay;
		ifs >> variableDescription;
		ifs >> randSEED;

		//VehicleType, nVehicle
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> nVehicle;
		//QCType, nQC, nATC
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> nQC;
		ifs >> variableDescription;
		ifs >> nATC;
		ifs >> variableDescription;
		ifs >> nShipBayPerQC;
		ifs >> variableDescription;
		ifs >> n20x20BayPerQC;
		ifs.close();
	}
	ifs.clear();

	//...apron specification
	ifs.open( "SPEC_Apron.txt" );
	if(	!ifs.fail() ){
		string variableDescription;
		ifs >> variableDescription;
		ifs >> berthLength;
		ifs >> variableDescription;
		ifs >> berthWidth;
		ifs >> variableDescription;
		ifs >> quayLength;
		ifs >> variableDescription;
		ifs >> quayWidth;
		ifs >> variableDescription;
		ifs >> middleLaneGap;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> boundaryOfQuaysideHP;
		ifs.close();
	}
	ifs.clear();
	
	//...quaycrane specification
	ifs.open( "SPEC_QC.txt" );
	if(	!ifs.fail() ){
		string variableDescription;
		ifs >> variableDescription;
		ifs >> nHPperQC;
		ifs >> variableDescription;
		ifs >> QCfrontReach;
		ifs >> variableDescription;
		ifs >> variableDescription;//QCbackReach;
		ifs >> variableDescription;
		ifs >> variableDescription;//QClength;
		ifs >> variableDescription;
		ifs >> variableDescription;//QCwidth;
		ifs >> variableDescription;
		ifs >> QClegWidth;
		ifs.close();
	}
	ifs.clear();

	//...automated stacking crane specification
	ifs.open( "SPEC_ASC.txt" );
	if(	!ifs.fail() ){
		string variableDescription;
		ifs >> variableDescription;
		ifs >> nHPperBlock;
		ifs >> variableDescription;
		ifs >> nRowperBlock;
		ifs >> variableDescription;
		ifs >> variableDescription;//blockLength;
		ifs >> variableDescription;
		ifs >> blockWidth;
		ifs.close();		
	}
	ifs.clear();

	//...container size;
	ifs.open( "SPEC_Container.txt" );
	if(	!ifs.fail() ){
		string variableDescription;
		ifs >> variableDescription;
		ifs >> Container40ftWidth;
		ifs >> variableDescription;
		ifs >> Container40ftLength;
		ifs >> variableDescription;
		ifs >> Container40ftHalfWidth;
		ifs >> variableDescription;
		ifs >> Container40ftHalfLength;
		ifs >> variableDescription;
		ifs >> Container20ftWidth;
		ifs >> variableDescription;
		ifs >> Container20ftLength;
		ifs >> variableDescription;
		ifs >> Container20ftHalfWidth;
		ifs >> variableDescription;
		ifs >> Container20ftHalfLength;
		ifs.close();
	}
	ifs.clear();	
	
}

CSimulationSpec::~CSimulationSpec(void)
{
}
