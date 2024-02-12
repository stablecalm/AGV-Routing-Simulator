#pragma once

#define _pi 3.1415926535

class CCraneSpec
{
public:
	//�ùķ��̼� ���� ��ġ
	double	unitTime;
	int		delay;
	int		randSEED;

	//QC ����
	ETrolleyType	trolleyType;
	ESpreaderType	spreaderType;

	//QC���� ��ġ
	int		nHPperQC;
	int		QClegWidth;		//QC�� �ٸ� �ʺ�
	int		QClegGap;		//QC�� �ٸ� ���� ƴ
	int		QCwidth;		//QC�� �ʺ�
	int		QClength;		//QC�� ����
	int		QCfrontReach;	//QC�� �������� Ƣ�� ���� �ִ� ����
	int		QCbackReach;	//QC�� �Ⱥ����� Ƣ�� ���� �ִ� ����
	int		QCTrolleyLegGap;//500
	long	halfCycletime;			//QC�� Half cycletime;(��)

	//ASC & Block ���� ��ġ
	int		nHPperBlock;
	int		nRowperBlock;
	int		blockLength;	//��ϼ�������(���̴� �κи�)
	int		blockWidth;		//����� �ʺ�
	int		ASClegWidth;
	int		ASClegHalfWidth;
	int		ASClegGap;
	int		ASCLoadingTime;		//ASC�� �����̳ʸ� ���ø��µ� �ɸ��� �ð�
	int		ASCUnloadingTime;		//ASC�� �����̳ʸ� �����µ� �ɸ��� �ð�
	int		ASCTravelInterval;		//ASC�� �̵��Ÿ� cm/0.1��

	//Container
	int		Container40ftWidth;		//�������� 40ft �����̳� �ʺ�
	int		Container40ftLength;		//�������� 40ft �����̳� ����
	int		Container40ftHalfWidth;	//�������� 40ft �����̳� �ʺ�/2
	int		Container40ftHalfLength;	//�������� 40ft �����̳� ����/2

	int		Container20ftWidth;		//�������� 20ft �����̳� �ʺ�
	int		Container20ftLength;		//�������� 20ft �����̳� ����
	int		Container20ftHalfWidth;	//�������� 20ft �����̳� �ʺ�/2
	int		Container20ftHalfLength;	//�������� 20ft �����̳� ����/2


	


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
