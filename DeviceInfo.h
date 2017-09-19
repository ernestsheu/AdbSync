


#pragma once

#define DEF_SCRIPE_PATH _T("script")

typedef enum {

	DEV_MASTER = 0,
	DEV_SLAVE_1,
	DEV_SLAVE_2,
	DEV_SLAVE_3,
	DEV_SLAVE_4,

	TOTAL_DEVICE

} DEVICE_MAP_ID;


const CString DEVICE_NAME[TOTAL_DEVICE] = {
	_T("Master"),		
	_T("Slave #1"),
	_T("Slave #2"),
	_T("Slave #3"),
	_T("Slave #4"),

};

