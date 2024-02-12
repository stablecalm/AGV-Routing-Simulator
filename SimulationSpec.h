#pragma once
#include "ALVEnumeration.h"

class CSimulationSpec
{
public:
	//...for drawing
	int		berthLength;			//�����ϳ��� ����
	int		quayLength;				//�Ⱥ��� ����
	int		berthWidth;
	int		quayWidth;
	int		middleLaneGap;
	int		QCfrontReach;	//QC�� �������� Ƣ�� ���� �ִ� ����
	
	//...simulation setting	
	bool			showGraphicInterface;	//�ùķ��̼� ȭ���� �����ش�.
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
	bool			dualLoad;				//������ dual-load ���� ����

	//...QC setting
	int				nQC;
	int				nHPperQC;
	bool			doubleCycling;			//QC�� double-cycling ���� ����
	long			halfCycletime;			//QC�� Half cycletime;(��)

	//...�߰�
	int				QCHPGap;
	int				QClegWidth;
	int				boundaryOfQuaysideHP;

	//...ATC setting
	int				nATC;
	int				nHPperBlock;
	int				nRowperBlock;

	//...�߰�
	int				blockWidth;
	int				blockGap;
	int				blockHPGap;

	//HP Information
	//...HP size;
	int		hpWidth;
	int		hpHalfWidth;
	int		hpLength;
	int		hpHalfLength;

	int		i20ftHalfGap;			//HP�� ���̴� 20��Ʈ �����̳� �� ��

	//...container size;
	int		Container40ftWidth;		//�������� 40ft �����̳� �ʺ�
	int		Container40ftLength;		//�������� 40ft �����̳� ����
	int		Container40ftHalfWidth;	//�������� 40ft �����̳� �ʺ�/2
	int		Container40ftHalfLength;	//�������� 40ft �����̳� ����/2

	int		Container20ftWidth;		//�������� 20ft �����̳� �ʺ�
	int		Container20ftLength;		//�������� 20ft �����̳� ����
	int		Container20ftHalfWidth;	//�������� 20ft �����̳� �ʺ�/2
	int		Container20ftHalfLength;	//�������� 20ft �����̳� ����/2

public:
	CSimulationSpec(void);
	~CSimulationSpec(void);

	//�߰�
	int		GetXposBlockHP(){ return (blockWidth - middleLaneGap*(nHPperBlock-1))/2; };

	//int GetXposQC()	{ return (quayLength-berthLength)/3 + (int)berthLength/(nQC + 0.5); }		//������ QC�� x��ǥ
	//int GetQCGap()	{ return (int)berthLength/(nQC+0.5); }		//QC ������ ����
	int GetXposQC()	{ return (quayLength-berthLength) + 2500; }		//������ QC�� x��ǥ
	int GetQCGap()	{ return berthLength/(nQC); }		//QC ������ ����
	int GetYposQC() { return berthWidth - QCfrontReach; }		//QC�� y��ǥ
	
	int GetYposYard() { return berthWidth + quayWidth; }		//��ġ���� ����� y��ǥ
	int GetXposYard()	{ return (quayLength - ( (blockWidth+blockGap)*nATC - blockGap ))/2; }	//������ ����� x��ǥ
	int	GetXposBlockTP()	{ return (blockWidth - middleLaneGap*(nHPperBlock-1))/2; }	//����� ��ǥ�� �������� ������ TP�� ��� x��ǥ



};
