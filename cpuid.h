#pragma once

#define ERROR_CPUID_CREATE_INSTANCE     0xE4000001
#define ERROR_CPUID_INIT                0xE4000002

class cpuid {


public:
	~cpuid();

	bool isOK() const
	{
		return instance != NULL;
	}

	DWORD init();
	long read_cpu_temperature();
	void destroy();

private:

	bool cpuid_create_instance();
	bool cpuid_init(const char* base_path);
	void cpuid_close();
	void cpuid_destroy_instance();
	void cpuid_refresh_info();
	int  cpuid_get_device_count();
	int  cpuid_get_device_class(int index);
	std::string cpuid_get_device_name(int index);
	int  cpuid_get_sensors_count(int index, int sensor_class);
	bool cpuid_get_sensor_infos(int index, int sensor_index, int sensor_class, int& sensor_id, std::string& name, int& raw_value, float& value, float& min, float& max);

	typedef void* (STDMETHODCALLTYPE *TQI)(UINT);
	TQI dll_qi = NULL;
	HMODULE dll_handle = NULL;
	void* instance = NULL;
	bool initialized = false;
	int cpu_index = -1;
};