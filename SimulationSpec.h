#pragma once
#include "ALVEnumeration.h"

class CSimulationSpec
{
public:
	//...for drawing
	int		berthLength;			//선석하나의 길이
	int		quayLength;				//안벽의 길이
	int		berthWidth;
	int		quayWidth;
	int		middleLaneGap;
	int		QCfrontReach;	//QC가 선석위로 튀어 나와 있는 길이
	
	//...simulation setting	
	bool			showGraphicInterface;	//시뮬레이션 화면을 보여준다.
	int				delay;
	double			unitTime;	
	bool			autostart;
	int				randSEED;
	ECargoType		cargoType;

	//...process settings
	int				nShipBayPerQC;
	int				n20x20BayPerQC;
	
	//...parameter setting
	int				avgWaitingTime;

	//...vehicle settings
	int				nVehicle;
	EDispatchRule	dispType;
	bool			dualLoad;				//차량의 dual-load 지원 여부

	//...QC setting
	int				nQC;
	int				nHPperQC;
	bool			doubleCycling;			//QC의 double-cycling 지원 여부
	long			halfCycletime;			//QC의 Half cycletime;(초)

	//...추가
	int				QCHPGap;
	int				QClegWidth;
	int				boundaryOfQuaysideHP;

	//...ATC setting
	int				nATC;
	int				nHPperBlock;
	int				nRowperBlock;

	//...추가
	int				blockWidth;
	int				blockGap;
	int				blockHPGap;

	//HP Information
	//...HP size;
	int		hpWidth;
	int		hpHalfWidth;
	int		hpLength;
	int		hpHalfLength;

	int		i20ftHalfGap;			//HP에 놓이는 20피트 컨테이너 간 갭

	//...container size;
	int		Container40ftWidth;		//수직기준 40ft 컨테이너 너비
	int		Container40ftLength;		//수직기준 40ft 컨테이너 길이
	int		Container40ftHalfWidth;	//수직기준 40ft 컨테이너 너비/2
	int		Container40ftHalfLength;	//수직기준 40ft 컨테이너 길이/2

	int		Container20ftWidth;		//수직기준 20ft 컨테이너 너비
	int		Container20ftLength;		//수직기준 20ft 컨테이너 길이
	int		Container20ftHalfWidth;	//수직기준 20ft 컨테이너 너비/2
	int		Container20ftHalfLength;	//수직기준 20ft 컨테이너 길이/2

public:
	CSimulationSpec(void);
	~CSimulationSpec(void);

	//추가
	int		GetXposBlockHP(){ return (blockWidth - middleLaneGap*(nHPperBlock-1))/2; };

	//int GetXposQC()	{ return (quayLength-berthLength)/3 + (int)berthLength/(nQC + 0.5); }		//최좌측 QC의 x좌표
	//int GetQCGap()	{ return (int)berthLength/(nQC+0.5); }		//QC 사이의 간격
	int GetXposQC()	{ return (quayLength-berthLength) + 2500; }		//최좌측 QC의 x좌표
	int GetQCGap()	{ return berthLength/(nQC); }		//QC 사이의 간격
	int GetYposQC() { return berthWidth - QCfrontReach; }		//QC의 y좌표
	
	int GetYposYard() { return berthWidth + quayWidth; }		//장치장의 블록의 y좌표
	int GetXposYard()	{ return (quayLength - ( (blockWidth+blockGap)*nATC - blockGap ))/2; }	//최좌측 블록의 x좌표
	int	GetXposBlockTP()	{ return (blockWidth - middleLaneGap*(nHPperBlock-1))/2; }	//블록의 좌표를 기준으로 최좌측 TP의 상대 x좌표



};
