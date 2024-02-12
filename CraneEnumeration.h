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

enum ECraneWorkState{		// QC나 ATC의 상태를 나타낸다.
	CWS_Idle,
	CWS_LoadedTravel,
	CWS_EmptyTravel,
	CWS_Pickup,
	CWS_WaitingforPickup,	//QC가 컨테이너를 가지러 Backreach까지 간 후, TP의 PickUp 가능상태를 기다림 (여기서 QC 지연 발생)
	CWS_TravelforPickup,	//QC가 컨테이너를 가지러 Backreach에서 TP까지 이동
	CWS_WaitingforDropoff,	//QC가 컨테이너를 들고 Backreach까지 간 후, TP의 Dropoff 가능상태를 기다림 (여기서 QC 지연 발생)
	CWS_TravelforDropoff,	//QC가 컨테이너를 들고 Backreach에서 TP까지 이동
	CWS_Dropoff,
	CWS_Hoisting,
	CWS_Traveling,
	CWS_Waiting
};