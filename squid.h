/////////////////////////////// Macros / Defines //////////////////////////////

#pragma once

#ifndef __SQUID_H__
#define __SQUID_H__


////////////////////////////// Includes ///////////////////////////////////////

#include "ntserv.h"
#include "ntservServiceControlManager.h"

#include "cpuid.h"
#include "kraken.h"

////////////////////////////// Classes ////////////////////////////////////////

class CMyService : public CNTService {
public:
	CMyService();
	virtual void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
	virtual void OnStop();
	virtual void OnPause();
	virtual void OnContinue();
	virtual void ShowHelp();
	virtual DWORD OnPowerEvent(DWORD dwEventType, LPVOID lpEventData);
	virtual ~CMyService();

protected:
	volatile long m_lWantStop;
	volatile long m_lPaused;
	volatile long m_lSuspended;
	cpuid         m_cpuid;
	kraken        m_kraken;
	HANDLE        m_event;

private:
	BOOL ReadSettings();

	// Temperatures from 0 to 100 in 5 degrees steps
	enum {PROFILE_SIZE = 20};
	// Intervals [0-5), [5-10) ...
	int  CalculateSpeed(long temperature);

	UINT          m_poolInterval;
	UINT          m_hysteresis;
	BYTE          m_profile[PROFILE_SIZE];
};

#endif //#ifndef __SQUID_H__