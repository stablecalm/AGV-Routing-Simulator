#pragma once

using namespace std;

//...qc trolley
enum ETrolleyType{
	Trolley_Undefined,
	Trolley_Single,
	Trolley_Double,
};

//...qc spreader
enum ESpreaderType{
	Spreader_Undefined,
	Spreader_Single,	//Single spreader
	Spreader_Twin,		//Twin spreader
	Spreader_TandemTwin
};

//...deadline of qc job
enum EDischargeDeadline{
	Tight_Deadline,
	Loose_Deadline
};

enum EBayType{
	BayType_20ft,
	BayType_40ft
};

enum ECraneWorkState{		// QC�� ATC�� ���¸� ��Ÿ����.
	CWS_Idle,
	CWS_LoadedTravel,
	CWS_EmptyTravel,
	CWS_Pickup,
	CWS_WaitingforPickup,	//QC�� �����̳ʸ� ������ Backreach���� �� ��, TP�� PickUp ���ɻ��¸� ��ٸ� (���⼭ QC ���� �߻�)
	CWS_TravelforPickup,	//QC�� �����̳ʸ� ������ Backreach���� TP���� �̵�
	CWS_WaitingforDropoff,	//QC�� �����̳ʸ� ��� Backreach���� �� ��, TP�� Dropoff ���ɻ��¸� ��ٸ� (���⼭ QC ���� �߻�)
	CWS_TravelforDropoff,	//QC�� �����̳ʸ� ��� Backreach���� TP���� �̵�
	CWS_Dropoff,
	CWS_Hoisting,
	CWS_Traveling,
	CWS_Waiting
};