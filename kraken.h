#pragma once

#define ERROR_KRAKEN_NOT_FOUND     0xE4001001
#define ERROR_KRAKEN_OPEN          0xE4001002

class kraken {
public:

	DWORD init();
	bool isOK() const
	{
		return initialized && (index >= 0);
	}
	bool update(int speed_percent, long& water_temperature, int& pump_speed, int& fan_speed) const;
	void destroy();

private:

	int get_device_index() const;

	int index = -1;
	bool initialized = false;
	HANDLE hdev = INVALID_HANDLE_VALUE;
};
