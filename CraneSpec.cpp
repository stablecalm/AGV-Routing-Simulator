#include "StdAfx.h"
#include "CraneSpec.h"

CCraneSpec::CCraneSpec(void)
{
	halfCycletime = 60;		//시간 당 30 box 처리한다고 가정 - Single cycle은 120초

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
		//vehicle 관련 변수 스킵
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		//QCType, nQC, nATC
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
		ifs >> variableDescription;
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
		ifs >> QCbackReach;
		ifs >> variableDescription;
		ifs >> QClength;
		ifs >> variableDescription;
		ifs >> QCwidth;
		ifs >> variableDescription;
		ifs >> QClegWidth;
		ifs >> variableDescription;
		ifs >> QClegGap;
		ifs >> variableDescription;
		ifs >> QCTrolleyLegGap;
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
		ifs >> blockLength;
		ifs >> variableDescription;
		ifs >> blockWidth;
		ifs >> variableDescription;
		ifs >> ASClegWidth;
		ifs >> variableDescription;
		ifs >> ASClegHalfWidth;
		ifs >> variableDescription;
		ifs >> ASClegGap;
		ifs >> variableDescription;
		ifs >> ASCLoadingTime;
		ifs >> variableDescription;
		ifs >> ASCUnloadingTime;
		ifs >> variableDescription;
		ifs >> ASCTravelInterval;
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

CCraneSpec::~CCraneSpec(void)
{
}
