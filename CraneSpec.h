#pragma once

#define _pi 3.1415926535

class CCraneSpec
{
public:
	//시뮬레이션 관련 수치
	double	unitTime;
	int		delay;
	int		randSEED;

	//QC 정의
	ETrolleyType	trolleyType;
	ESpreaderType	spreaderType;

	//QC관련 수치
	int		nHPperQC;
	int		QClegWidth;		//QC의 다리 너비
	int		QClegGap;		//QC의 다리 사이 틈
	int		QCwidth;		//QC의 너비
	int		QClength;		//QC의 길이
	int		QCfrontReach;	//QC가 선석위로 튀어 나와 있는 길이
	int		QCbackReach;	//QC가 안벽위로 튀어 나와 있는 길이
	int		QCTrolleyLegGap;//500
	long	halfCycletime;			//QC의 Half cycletime;(초)

	//ASC & Block 관련 수치
	int		nHPperBlock;
	int		nRowperBlock;
	int		blockLength;	//블록수직길이(보이는 부분만)
	int		blockWidth;		//블록의 너비
	int		ASClegWidth;
	int		ASClegHalfWidth;
	int		ASClegGap;
	int		ASCLoadingTime;		//ASC가 컨테이너를 들어올리는데 걸리는 시간
	int		ASCUnloadingTime;		//ASC가 컨테이너를 내리는데 걸리는 시간
	int		ASCTravelInterval;		//ASC의 이동거리 cm/0.1초

	//Container
	int		Container40ftWidth;		//수직기준 40ft 컨테이너 너비
	int		Container40ftLength;		//수직기준 40ft 컨테이너 길이
	int		Container40ftHalfWidth;	//수직기준 40ft 컨테이너 너비/2
	int		Container40ftHalfLength;	//수직기준 40ft 컨테이너 길이/2

	int		Container20ftWidth;		//수직기준 20ft 컨테이너 너비
	int		Container20ftLength;		//수직기준 20ft 컨테이너 길이
	int		Container20ftHalfWidth;	//수직기준 20ft 컨테이너 너비/2
	int		Container20ftHalfLength;	//수직기준 20ft 컨테이너 길이/2


	


public:
	CCraneSpec(void);
	~CCraneSpec(void);

	static double normalRandom( double mean, double stdDev )
	{
		double uValue, vValue, rValue, x;
		do 
		{
			uValue = (double)rand()/(RAND_MAX+1);
			vValue = (double)rand()/(RAND_MAX+1);
			rValue = stdDev*sqrt( -2.0 * log(1-uValue) );
			x = rValue*sin( 2.0 * _pi * vValue );
		} while( (x > 2.0*stdDev)||(x < -2.0*stdDev) );

		return x + mean;
	}

};
