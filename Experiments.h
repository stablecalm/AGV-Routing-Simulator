#pragma once

#include "ProcessManager.h"
#include "ALVManager.h"
#include "YTEmulator\ALVEmulator.h"

using namespace std;

class CProcessManager;
class CALVManager;

//실험 수행을 위한 Wrapper Class
class CExperiments
{	
private:
	CALVEmulator*		m_pALVEmulator;
	CProcessManager*	m_pProcessManager;
	CALVManager*	m_pEquipmentManager;

public:
	CExperiments(void);
	~CExperiments(void);

	void Initialize();
	void StartExperiments();
	
	CALVEmulator*		GetALVEmulator(){	return m_pALVEmulator; }
	CALVManager*	GetAHVSupervisor(){	return m_pEquipmentManager; }

};
