#include "stdafx.h"

#include "usbexpress/SiUSBXp.h"

#include "kraken.h"

int kraken::get_device_index() const
{
	DWORD count = 0;

	auto res = SI_GetNumDevices(&count);
	if (res != SI_SUCCESS) {
		return -1;
	}

	SI_DEVICE_STRING devStr;
	std::string vid, pid;

	for (DWORD i = 0; i < count; ++i) {
		res = SI_GetProductString(i, devStr, SI_RETURN_VID);
		if (res == SI_SUCCESS) {
			vid = devStr;
		}
		res = SI_GetProductString(i, devStr, SI_RETURN_PID);
		if (res == SI_SUCCESS) {
			pid = devStr;
		}
		if ((0 == _stricmp(vid.c_str(), "2433")) && (0 == _stricmp(pid.c_str(), "b200"))) {
			// Use first suitable Kraken device
			return (int)i;
		}
	}
	return -1;
}

bool kraken::update(int speed_percent, long& water_temperature, int& pump_speed, int& fan_speed) const
{
	bool res = false;

	if (!isOK()) {
		return res;
	}

	auto err = SI_FlushBuffers(hdev, 1, 16);
	if (err != SI_SUCCESS) {
		ATLTRACE(_T("kraken update SI_FlushBuffers: %d"), err);
		return res;
	}

	// Minimal speed cannot be less than 30%
	BYTE percent = (BYTE)max(speed_percent, 30);

	DWORD bytes_written = 0;
	BYTE buf_write[2];

	// pump
	buf_write[0] = 0x13;
	buf_write[1] = percent;

	err = SI_Write(hdev, buf_write, 2, &bytes_written, NULL);
	if (err != SI_SUCCESS) {
		ATLTRACE(_T("kraken update pump SI_Write failed: %d %d"), err, bytes_written);
		return res;
	}
	Sleep(200);

	// fan
	buf_write[0] = 0x12;
	buf_write[1] = percent;

	err = SI_Write(hdev, buf_write, 2, &bytes_written, NULL);
	if (err != SI_SUCCESS) {
		ATLTRACE(_T("kraken update fan SI_Write failed: %d %d"), err, bytes_written);
		return res;
	}
	Sleep(100);

	DWORD bytes_in_queue = 0;
	DWORD queue_status = 0;

	err = SI_CheckRXQueue(hdev, &bytes_in_queue, &queue_status);
	if (err != SI_SUCCESS) {
		ATLTRACE(_T("kraken update SI_CheckRXQueue failed: %d %d %d"), err, bytes_in_queue, queue_status);
		return res;
	}
	if (queue_status != SI_RX_READY) {
		ATLTRACE(_T("kraken update SI_CheckRXQueue queue not ready: %d %d %d"), err, bytes_in_queue, queue_status);
		return res;
	}

	ATLASSERT(bytes_in_queue <= 32);

	BYTE buf_read[32] = {0};
	DWORD bytes_read = 0;

	err = SI_Read(hdev, buf_read, bytes_in_queue, &bytes_read, NULL);
	if (err != SI_SUCCESS) {
		ATLTRACE(_T("kraken update SI_Read failed: %d %d %d"), err, bytes_in_queue, bytes_read);
		return res;
	}

	res = true;

	fan_speed = (int)buf_read[0] * 256 + (int)buf_read[1];
	pump_speed = (int)buf_read[8] * 256 + (int)buf_read[9];

	float temperature = (float)buf_read[10] + (float)buf_read[14] / 10.0f;
	water_temperature = lround((double)temperature * 100.0);

	ATLTRACE(_T("kraken update: %f %d %d\n"), temperature, fan_speed, pump_speed);

	return res;
}

DWORD kraken::init()
{
	if (index < 0) {
		index = get_device_index();
		if (index < 0) {
			return ERROR_KRAKEN_NOT_FOUND;
		}
	}

	if (hdev == INVALID_HANDLE_VALUE) {
		SI_SetTimeouts(1000, 1000);
		auto res = SI_Open((DWORD)index, &hdev);
		if (res != SI_SUCCESS) {
			hdev = INVALID_HANDLE_VALUE;
			return ERROR_KRAKEN_OPEN;
		}
		Sleep(200);
	}
	initialized = true;
	return ERROR_SUCCESS;
}

void kraken::destroy()
{
	if (hdev != INVALID_HANDLE_VALUE) {
		SI_Close(hdev);
		Sleep(150);
		hdev = INVALID_HANDLE_VALUE;
	}
	initialized = false;
}
