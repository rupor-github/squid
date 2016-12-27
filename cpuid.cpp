#include "stdafx.h"

#include "cpuid.h"

#ifdef _WIN64
#define CPUIDSDK _T("CPUIDSDK64")
#define CPUIDSDKDLL "cpuidsdk64.dll"
#else
#define CPUIDSDK _T("CPUIDSDK")
#define CPUIDSDKDLL "cpuidsdk.dll"
#endif

const UINT CLASS_DEVICE_PROCESSOR   = 4;
const UINT SENSOR_CLASS_TEMPERATURE = 512;
const UINT SENSOR_TEMPERATURE_CPU   = 16777728;

cpuid::~cpuid()
{
	destroy();
}

bool cpuid::cpuid_create_instance()
{
	typedef void* (STDMETHODCALLTYPE *TPROC_CREATE_INSTANCE)(void);
	TPROC_CREATE_INSTANCE dll_ci = NULL;

	if ((dll_ci = (TPROC_CREATE_INSTANCE)dll_qi(909517561)) == NULL) {
		ATLTRACE(_T("CPUID QI CREATE_INSTANCE failed\n"));
		return false;
	}
	if ((instance = dll_ci()) == NULL) {
		ATLTRACE(_T("CPUID CREATE_INSTANCE failed\n"));
		return false;
	}
	return true;
}

bool cpuid::cpuid_init(const char* base_path)
{
	typedef int (STDMETHODCALLTYPE *TPROC_INIT)(void*, const char*, const char*, UINT, DWORD*, DWORD*);
	TPROC_INIT dll_init = NULL;

	if ((dll_init = (TPROC_INIT)dll_qi(1379035511)) == NULL) {
		ATLTRACE(_T("CPUID QI INIT failed\n"));
		return false;
	}
	DWORD err = 0, err2 = 0;
	if (dll_init(instance, base_path, CPUIDSDKDLL, UINT_MAX, &err, &err2) != 1) {
		ATLTRACE(_T("CPUID INIT failed: %x %x\n"), err, err2);
		return false;
	}
	return true;
}

void cpuid::cpuid_close()
{
	typedef void (STDMETHODCALLTYPE *TPROC_CLOSE)(void*);
	TPROC_CLOSE dll_close = NULL;

	if ((dll_close = (TPROC_CLOSE)dll_qi(1197983634)) == NULL) {
		ATLTRACE(_T("CPUID QI CLOSE failed\n"));
		return;
	}
	dll_close(instance);
}

void cpuid::cpuid_destroy_instance()
{
	typedef void (STDMETHODCALLTYPE *TPROC_DESTROY_INSTANCE)(void*);
	TPROC_DESTROY_INSTANCE dll_di = NULL;

	if ((dll_di = (TPROC_DESTROY_INSTANCE)dll_qi(438593575)) == NULL) {
		ATLTRACE(_T("CPUID QI DESTROY_INSTANCE failed\n"));
		return;
	}
	dll_di(instance);
	instance = NULL;
}

void cpuid::cpuid_refresh_info()
{
	typedef void (STDMETHODCALLTYPE *TPROC_REFRESH_INFO)(void*);
	TPROC_REFRESH_INFO dll_refresh = NULL;

	if ((dll_refresh = (TPROC_REFRESH_INFO)dll_qi(302217561)) == NULL) {
		ATLTRACE(_T("CPUID QI REFRESH_INFO failed\n"));
		return;
	}
	dll_refresh(instance);
}

int cpuid::cpuid_get_device_count()
{
	typedef int (STDMETHODCALLTYPE *TPROC_GETDEVICECOUNT)(void*);
	TPROC_GETDEVICECOUNT dll_getdevcount = NULL;

	if ((dll_getdevcount = (TPROC_GETDEVICECOUNT)dll_qi(621114419)) == NULL) {
		ATLTRACE(_T("CPUID QI GETDEVICECOUNT failed\n"));
		return -1;
	}
	return dll_getdevcount(instance);
}

int cpuid::cpuid_get_device_class(int index)
{
	typedef int (STDMETHODCALLTYPE *TPROC_GETDEVICECLASS)(void*, int);
	TPROC_GETDEVICECLASS dll_getdevclass = NULL;

	if ((dll_getdevclass = (TPROC_GETDEVICECLASS)dll_qi(914388262)) == NULL) {
		ATLTRACE(_T("CPUID QI GETDEVCLASS failed\n"));
		return -1;
	}
	return dll_getdevclass(instance, index);
}

std::string cpuid::cpuid_get_device_name(int index)
{
	typedef BSTR(STDMETHODCALLTYPE *TPROC_GETDEVICENAME)(void*, int);
	TPROC_GETDEVICENAME dll_getdevname = NULL;

	if ((dll_getdevname = (TPROC_GETDEVICENAME)dll_qi(961816339)) == NULL) {
		ATLTRACE(_T("CPUID QI GETDEVICENAME failed\n"));
		return "";
	}
	BSTR bstr = dll_getdevname(instance, index);
	std::string name((const char*)bstr);
	SysFreeString(bstr);
	return name;
}

int cpuid::cpuid_get_sensors_count(int index, int sensor_class)
{
	typedef int (STDMETHODCALLTYPE *TPROC_GETSENSORSCOUNT)(void*, int, int);
	TPROC_GETSENSORSCOUNT dll_getsensorscount = NULL;

	if ((dll_getsensorscount = (TPROC_GETSENSORSCOUNT)dll_qi(2506266000)) == NULL) {
		ATLTRACE(_T("CPUID QI GETSENSORSCOUNT failed\n"));
		return -1;
	}
	return dll_getsensorscount(instance, index, sensor_class);
}

bool cpuid::cpuid_get_sensor_infos(int index, int sensor_index, int sensor_class, int& sensor_id, std::string& name, int& raw_value, float& value, float& min_value, float& max_value)
{
	typedef int (STDMETHODCALLTYPE *TPROC_GETSENSORINFOS)(void*, int, int, int, int*, BSTR*, int*, float*, float*, float*);

	TPROC_GETSENSORINFOS dll_getsensorinfos = NULL;
	if ((dll_getsensorinfos = (TPROC_GETSENSORINFOS)dll_qi(1230254370)) == NULL) {
		ATLTRACE(_T("CPUID QI GETSENSORINFOS failed\n"));
		return false;
	}

	BSTR bstr = NULL;
	if (dll_getsensorinfos(instance, index, sensor_index, sensor_class, &sensor_id, &bstr, &raw_value, &value, &min_value, &max_value) != 1) {
		ATLTRACE(_T("CPUID GETSENSORINFOS failed\n"));
		return false;
	}
	name = (const char*)bstr;
	SysFreeString(bstr);
	
	return true;
}

// I am repeating CAM's logic here however strange it may be
long cpuid::read_cpu_temperature()
{
	int res = 0;

	if (!isOK()) {
		return res;
	}

	cpuid_refresh_info();

	if (cpu_index == -1) {
		int numberOfDevices = cpuid_get_device_count();
		for (int i = 0; i < numberOfDevices; ++i) {
			if (cpuid_get_device_class(i) == CLASS_DEVICE_PROCESSOR) {
				cpu_index = i;
				break;
			}
		}
	}

	if (cpu_index == -1) {
		return res;
	}

	auto name = cpuid_get_device_name(cpu_index);
	auto numberOfSensors = cpuid_get_sensors_count(cpu_index, SENSOR_CLASS_TEMPERATURE);

	ATLTRACE(_T("CPU: %S\tSensors: %d\n"), name.c_str(), numberOfSensors);

	double sum = 0.0;
	int    count = 0;

	for (auto i = 0; i < numberOfSensors; ++i) {

		int   sensor_id = 0;
		int   raw_value = 0;
		float value     = 0.0;
		float min_value = 0.0;
		float max_value = 0.0;

		if (cpuid_get_sensor_infos(cpu_index, i, SENSOR_CLASS_TEMPERATURE, sensor_id, name, raw_value, value, min_value, max_value) && (value > 0.0)) {

			ATLTRACE(_T("\tSensor #%d - %S [id: %d, raw: %d, (%f, %f, %f)] \n"), i, name.c_str(), sensor_id, raw_value, value, min_value, max_value);

			if ((name.find("Core") != std::string::npos) && (value > 10.0f) && (value < 120.0f)) {
				sum += value;
				count++;
			}
		}
	}
	if (count > 0) {
		res = lround(sum * 100.0 / (double)count);
	}
	return res;
}

DWORD cpuid::init()
{
	TCHAR szAppPath[_MAX_PATH];
	szAppPath[0] = _T('\0');

	if (!GetModuleFileName(NULL, szAppPath, _MAX_PATH)) {
		return GetLastError();
	}

	TCHAR szPath[_MAX_PATH];
	szPath[0] = _T('\0');
	TCHAR szDrive[_MAX_DRIVE];
	szDrive[0] = _T('\0');
	TCHAR szDir[_MAX_DIR];
	szDir[0] = _T('\0');
	TCHAR szFname[_MAX_FNAME];
	szFname[0] = _T('\0');

	_tsplitpath_s(szAppPath, szDrive, sizeof(szDrive) / sizeof(TCHAR), szDir, sizeof(szDir) / sizeof(TCHAR), NULL, 0, NULL, 0);
	_tmakepath_s(szPath, sizeof(szPath) / sizeof(TCHAR), szDrive, szDir, CPUIDSDK, _T("DLL"));

	dll_handle = LoadLibrary(szPath);
	if (dll_handle == NULL) {
		return GetLastError();
	}

	dll_qi = (TQI)GetProcAddress(dll_handle, "QueryInterface");
	if (dll_qi == NULL) {
		return GetLastError();
	}

	if (!cpuid_create_instance()) {
		ATLTRACE(_T("CPUID CREATE_INSTANCE failed\n"));
		return ERROR_CPUID_CREATE_INSTANCE;
	}

	_tmakepath_s(szPath, sizeof(szPath) / sizeof(TCHAR), szDrive, szDir, NULL, NULL);
	if (!cpuid_init(CT2A(szPath))) {
		ATLTRACE(_T("CPUID CREATE_INSTANCE failed\n"));
		return ERROR_CPUID_INIT;
	}

	return ERROR_SUCCESS;
}

void cpuid::destroy()
{
	if (instance != NULL) {
		cpuid_close();
		cpuid_destroy_instance();
	}
	if (dll_handle != NULL) {
		FreeLibrary(dll_handle);
		dll_qi = NULL;
		dll_handle = NULL;
	}
}

