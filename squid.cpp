#include "stdafx.h"
#include "ntserv_msg.h"
#include "ntservEventLog.h"
#include "PerfCtrv2Mgr.h"
#include "perfcountersv2.h"

#include "squid.h"

#ifdef _DEBUG
BOOL CALLBACK EnumServices(void* /*pData*/, ENUM_SERVICE_STATUS& Service)
#else
BOOL CALLBACK EnumServices(void* /*pData*/, ENUM_SERVICE_STATUS& /*Service*/)
#endif //#ifdef _DEBUG
{
#ifdef _DEBUG
	ATLTRACE(_T("Service name is %s\n"), Service.lpServiceName);
	ATLTRACE(_T("Friendly name is %s\n"), Service.lpDisplayName);
#endif //#ifdef _DEBUG

	return TRUE; //continue enumeration
}

#ifdef _DEBUG
BOOL CALLBACK EnumServices2(void* /*pData*/, ENUM_SERVICE_STATUS_PROCESS& ssp)
#else
BOOL CALLBACK EnumServices2(void* /*pData*/, ENUM_SERVICE_STATUS_PROCESS& /*ssp*/)
#endif
{
#ifdef _DEBUG
	ATLTRACE(_T("Service name is %s\n"), ssp.lpServiceName);
	ATLTRACE(_T("Friendly name is %s\n"), ssp.lpDisplayName);
#endif

	return TRUE; //continue enumeration
}

int _tmain(int /*argc*/, TCHAR* /*argv*/[], TCHAR* /*envp*/[])
{
	//All that is required to get the service up and running
	CNTServiceCommandLineInfo cmdInfo;
	CMyService Service;
	Service.SetAllowCommandLine(TRUE);
	Service.SetAllowNameChange(TRUE);
	Service.SetAllowDescriptionChange(TRUE);
	Service.SetAllowDisplayNameChange(TRUE);
	Service.ParseCommandLine(cmdInfo);
	return Service.ProcessShellCommand(cmdInfo);
}

CMyService::CMyService() :
	CNTService(_T("Squid"),
		_T("Squid service for NZXT Kraken"),
		SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_POWEREVENT,
		_T("Monitors CPU core temperature and controls Kraken's pump and fan speeds")),
	m_lWantStop(FALSE),
	m_lPaused(FALSE),
	m_lSuspended(FALSE),
	m_event(NULL),
	m_poolInterval(1500),
	m_hysteresis(1)
{
	m_dwStartType = SERVICE_AUTO_START;
	m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CMyService::~CMyService()
{
	if (m_event != NULL) {
		CloseHandle(m_event);
		m_event = NULL;
	}
}

BOOL CMyService::ReadSettings()
{
	BYTE profile_performance[PROFILE_SIZE] = {50,50,50,50,50,50,65,75,76,77,78,80,100,100,100,100,100,100,100,100};
	BYTE profile_silent[PROFILE_SIZE] = {25,25,25,25,25,25,25,25,30,35,55,75,100,100,100,100,100,100,100,100};
	BYTE profile_fixed[PROFILE_SIZE] = {45,45,45,45,45,45,45,45,50,50,55,75,100,100,100,100,100,100,100,100};

	DWORD dwErr = 0;

	m_poolInterval = GetProfileInt(_T("General"), _T("PoolInterval"), m_poolInterval, &dwErr);
	if (dwErr != ERROR_SUCCESS) {
		if (!WriteProfileInt(_T("General"), _T("PoolInterval"), m_poolInterval)) {
			return FALSE;
		}
	}

	m_hysteresis = GetProfileInt(_T("General"), _T("Hysteresis"), m_hysteresis, &dwErr);
	if (dwErr != ERROR_SUCCESS) {
		if (!WriteProfileInt(_T("General"), _T("Hysteresis"), m_hysteresis)) {
			return FALSE;
		}
	}

	auto name = GetProfileString(_T("General"), _T("Profile"), _T("Fixed"), &dwErr);
	if (dwErr != ERROR_SUCCESS) {
		if (!WriteProfileString(_T("General"), _T("Profile"), _T("Fixed"))) {
			return FALSE;
		}
		if (!WriteProfileBinary(_T("Profiles"), _T("Fixed"), profile_fixed, sizeof(profile_fixed))) {
			return FALSE;
		}
		if (!WriteProfileBinary(_T("Profiles"), _T("Performance"), profile_performance, sizeof(profile_performance))) {
			return FALSE;
		}
		if (!WriteProfileBinary(_T("Profiles"), _T("Silent"), profile_silent, sizeof(profile_silent))) {
			return FALSE;
		}
		memcpy(m_profile, profile_performance, sizeof(m_profile));
		return TRUE;
	}

	LPBYTE pData = NULL;
	ULONG  size = 0;
	if (!GetProfileBinary(_T("Profiles"), name.c_str(), &pData, &size) || (size != PROFILE_SIZE)) {
		if (pData != NULL) {
			delete[] pData;
		}
		if (!WriteProfileString(_T("General"), _T("Profile"), _T("Fixed"))) {
			return FALSE;
		}
		if (!WriteProfileBinary(_T("Profiles"), _T("Fixed"), profile_fixed, sizeof(profile_fixed))) {
			return FALSE;
		}
		if (!WriteProfileBinary(_T("Profiles"), _T("Performance"), profile_performance, sizeof(profile_performance))) {
			return FALSE;
		}
		if (!WriteProfileBinary(_T("Profiles"), _T("Silent"), profile_silent, sizeof(profile_silent))) {
			return FALSE;
		}
		memcpy(m_profile, profile_fixed, sizeof(m_profile));
		return TRUE;
	}

	memcpy(m_profile, pData, sizeof(m_profile));
	delete[] pData;
	return TRUE;
}

int CMyService::CalculateSpeed(long temperature)
{
	if (temperature <= 0) {
		return 40;
	}
	if (temperature >= 10000) {
		return 100;
	}
	return m_profile[temperature / 500];
}

void CMyService::ServiceMain(DWORD /*dwArgc*/, LPTSTR* /*lpszArgv*/)
{
	DWORD check_point = 0;

	ReportStatus(SERVICE_START_PENDING, ++check_point, 1000);

	if (m_event == NULL) {
		m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, _T("Unable to initialize service"));
		throw CNTServiceTerminateException(ERROR_SERVICE_SPECIFIC_ERROR, ERROR_ALREADY_EXISTS);
	}

	if (!ReadSettings()) {
		m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, _T("Unable to read service settings"));
		throw CNTServiceTerminateException(ERROR_SERVICE_SPECIFIC_ERROR, ERROR_WRITE_FAULT);
	}

	ReportStatus(SERVICE_START_PENDING, ++check_point, 1000);

	CPerfProvider PerfProvider;
	DWORD dwErr = PerfProvider.StartEx(&SquidPerfProviderGuid);
	if (dwErr != ERROR_SUCCESS) {
		m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, _T("Unable to initialize performance provider"));
		throw CNTServiceTerminateException(ERROR_SERVICE_SPECIFIC_ERROR, dwErr);
	}
	CPerfCounterSet PerfCounters;
	dwErr = PerfProvider.SetCounterSetInfo(&PERF_SQUIDInfo.CounterSet, sizeof(PERF_SQUIDInfo));
	if (dwErr != ERROR_SUCCESS) {
		m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, _T("Failed to set counter set info"));
		throw CNTServiceTerminateException(ERROR_SERVICE_SPECIFIC_ERROR, dwErr);
	}
	PerfCounters.Initialize(&PerfProvider);
	dwErr = PerfCounters.CreateInstance(&PERF_SQUIDGuid, L"", 1);
	if (dwErr != ERROR_SUCCESS) {
		m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, _T("Failed to create counter set instance"));
		throw CNTServiceTerminateException(ERROR_SERVICE_SPECIFIC_ERROR, dwErr);
	}

	ReportStatus(SERVICE_START_PENDING, ++check_point, 5000);

	dwErr = m_kraken.init();
	if (dwErr != ERROR_SUCCESS) {
		m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, _T("Unable to connect to NXZT kraken"));
		throw CNTServiceTerminateException(ERROR_SERVICE_SPECIFIC_ERROR, dwErr);
	}

	ReportStatus(SERVICE_START_PENDING, ++check_point, 30000);

	dwErr = m_cpuid.init();
	if (dwErr != ERROR_SUCCESS) {
		m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, _T("Unable to initialize cpuidsdk"));
		throw CNTServiceTerminateException(ERROR_SERVICE_SPECIFIC_ERROR, dwErr);
	}

	ReportStatus(SERVICE_RUNNING);

	m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_STARTED, m_sDisplayName.c_str());

	long temps[5] = {0};
	size_t last = 0;

	long lOldPause = m_lPaused;
	long lOldSuspend = m_lSuspended;
	long delay = 0;
	while (!m_lWantStop) {

		if (!m_lPaused && !m_lSuspended) {

			if (delay == 0) {

				if (!m_kraken.isOK()) {
					// After resuming operations we need to re-open USB device
					dwErr = m_kraken.init();
					if (dwErr != ERROR_SUCCESS) {
						m_EventLogSource.Report(EVENTLOG_ERROR_TYPE, _T("Unable to re-connect to NXZT kraken"));
					}
					// Start collecting fresh hysteresis data
					last = 0;
					memset(temps, 0, sizeof(temps));
				} else {
					long cpu_temperature = m_cpuid.read_cpu_temperature();
					if (cpu_temperature > 0) {

						PerfCounters.SetL(PERF_SQUID_CPUTEMP, cpu_temperature);

						temps[last++] = cpu_temperature;
						if (last == sizeof(temps) / sizeof(long)) {
							last = 0;
						}

						long long sum = 0;
						size_t count = 0;
						for (long e : temps) {
							if (e > 0) {
								sum += e;
								count++;
							}
						}
						if (count > 0) {
							sum /= count;
						}
						PerfCounters.SetL(PERF_SQUID_CPUTEMP_HYST, (long)sum);

						if (m_hysteresis) {
							cpu_temperature = (long)sum;
						}
					}
					long water_temperature = 0;
					int pump_speed = 0, fan_speed = 0;
					if (m_kraken.update(CalculateSpeed(cpu_temperature), water_temperature, pump_speed, fan_speed)) {
						PerfCounters.SetL(PERF_SQUID_WATERTEMP, water_temperature);
						PerfCounters.SetL(PERF_SQUID_PUMPSPEED, pump_speed);
						PerfCounters.SetL(PERF_SQUID_FANSPEED, fan_speed);
					}
				}

			} else {
				// After resuming wait several cycles before continuing
				--delay;
			}
		}

		// Wait for the specified time or until suspend signalled
		WaitForSingleObject(m_event, m_poolInterval);

		//We were notified about suspend/resume
		if (m_lSuspended != lOldSuspend) {
			if (m_lSuspended) {
				m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_SUSPENDED, m_sDisplayName.c_str());
				// Close USB device
				m_kraken.destroy();
			} else {
				m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_RESUMED, m_sDisplayName.c_str());
				delay = 5;
			}
		}
		lOldSuspend = m_lSuspended;

		//SCM has requested a Pause / Continue
		if (m_lPaused != lOldPause) {
			if (m_lPaused) {
				ReportStatus(SERVICE_PAUSED);

				//Report to the event log that the service has paused successfully
				m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_PAUSED, m_sDisplayName.c_str());
			} else {
				ReportStatus(SERVICE_RUNNING);

				//Report to the event log that the service has stopped continued
				m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_CONTINUED, m_sDisplayName.c_str());
			}
		}
		lOldPause = m_lPaused;
	}

	check_point = 0;
	ReportStatus(SERVICE_STOP_PENDING, ++check_point, 30000);
	m_cpuid.destroy();
	ReportStatus(SERVICE_STOP_PENDING, ++check_point, 5000);
	m_kraken.destroy();
	m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_STOPPED, m_sDisplayName.c_str());
}

DWORD CMyService::OnPowerEvent(DWORD dwEventType, LPVOID /*lpEventData*/)
{
	if (dwEventType == PBT_APMSUSPEND) {
		InterlockedExchange(&m_lSuspended, TRUE);
		SetEvent(m_event);
	} else if (dwEventType == PBT_APMRESUMESUSPEND) {
		InterlockedExchange(&m_lSuspended, FALSE);
	}
	return NO_ERROR;
}


void CMyService::OnStop()
{
	InterlockedExchange(&m_lWantStop, TRUE);
	SetEvent(m_event);
}

void CMyService::OnPause()
{
	InterlockedExchange(&m_lPaused, TRUE);
	SetEvent(m_event);
}

void CMyService::OnContinue()
{
	InterlockedExchange(&m_lPaused, FALSE);
	SetEvent(m_event);
}

void CMyService::ShowHelp()
{
	_tprintf(_T("Squid service for NZXT Kraken 1.0\nUsage: squidsrv [ -install | -uninstall | -remove | -start | -pause | -continue | -stop | -help | -? ]\n"));
}
