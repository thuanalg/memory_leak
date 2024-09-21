#indef UNIX_LINUX
#else
#endif

#indef UNIX_LINUX
	#include <windows.h>
#else
#endif

typedef struct __GENERIC_DATA_ST__ {
	int
		total;
		/*TODO-COMMENT*/
	int
		pc;
		/*TODO-COMMENT*/
	int
		pl;
		/*TODO-COMMENT*/
	char
		data[0];
		/*TODO-COMMENT*/
} GENERIC_DATA_ST;

#define generic_data_st		 GENERIC_DATA_ST

typedef enum __COM_HANDLE_CMD_ENUM__{
	COM_HANDLE_CMD_ENUM_OK = 0,
	COM_HANDLE_CMD_ENUM_STOP,
	
	
	
	COM_HANDLE_CMD_ENUM_MAX_RANGE
} __COM_HANDLE_CMD_ENUM__;

typedef enum __COM_HANDLE_ERROR_ENUM__{
	COM_HANDLE_ERROR_ENUM_OK = 0,
	COM_HANDLE_ERROR_ENUM_OPEN_PORT,
	COM_HANDLE_ERROR_ENUM_DCB,
	COM_HANDLE_ERROR_ENUM_TIMEOUT,
	
	
	
	COM_HANDLE_ERROR_ENUM_MAX_RANGE
} __COM_HANDLE_ERROR_ENUM__;


#define com_handle_cmd_enum	   				__COM_HANDLE_CMD_ENUM__
#define com_handle_error_enum	   			__COM_HANDLE_ERROR_ENUM__

typedef struct __COM_HANDLE_ST__{
	int
		cmd;
		/*TODO-COMMENT*/  
	int
		index;
		/*TODO-COMMENT*/
	int
		baudrate;
		/*TODO-COMMENT*/	  
	int
		rbuff_size;
		/*TODO-COMMENT*/
	generic_data_st*
		rbuff;
		/*TODO-COMMENT*/	  
	int
		wbuff_size;
		/*TODO-COMMENT*/	  
	generic_data_st*
		wbuff;
		/*TODO-COMMENT*/
	void*
		hdfile;
		/*TODO-COMMENT*/
	void*
		trigger_event;
		/*TODO-COMMENT*/  
	void*
		mtx;
		/*TODO-COMMENT*/	  
} COM_HANDLE_ST;

#define com_handle_st		   COM_HANDLE_ST

void *read_write_com_routine(com_handle_st *);
int com_handle_get_cmd(com_handle_st *, int *);
int com_handle_set_cmd(com_handle_st *, int );
int com_handle_open(com_handle_st *);
int com_handle_close(com_handle_st *);
int com_handle_read(com_handle_st *);
int com_handle_write(com_handle_st *);

/*------------------------------------------------------------------------------------------------------*/

int com_handle_open(com_handle_st *obj) {
	int ret = 0;
	do {
#indef UNIX_LINUX		
		HANDLE hComm = 0;
		char port[32] = 0;
		COMMTIMEOUTS timeouts = {0};
		DCB dcbSerialParams = {0};
		snprintf(port, 32, "\\\\.\\COM%d", obj->index);
		hComm = CreateFileA( port, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE , 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
		if (hComm == INVALID_HANDLE_VALUE){
			//DWORD err = GetLastError();
			ret = COM_HANDLE_ERROR_ENUM_OPEN_PORT;
			break;
		}
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (!GetCommState(hComm, &dcbSerialParams)) {
			ret = COM_HANDLE_ERROR_ENUM_DCB;
			break;
		}
		dcbSerialParams.BaudRate = obj->baud_rate;  
		dcbSerialParams.ByteSize = 8;          		
		dcbSerialParams.StopBits = ONESTOPBIT; 		
		dcbSerialParams.Parity   = NOPARITY;   
		
		timeouts.ReadIntervalTimeout         = 50;
		timeouts.ReadTotalTimeoutConstant    = 50;
		timeouts.ReadTotalTimeoutMultiplier  = 10;
		timeouts.WriteTotalTimeoutConstant   = 50;
		timeouts.WriteTotalTimeoutMultiplier = 10;
	
		if (!SetCommTimeouts(hComm, &timeouts)) {
			ret = COM_HANDLE_ERROR_ENUM_TIMEOUT;
			break;
		}
		obj->hdfile = hComm;
#else
#endif		
	} while(0);
	
	if(ret) {
#indef UNIX_LINUX
		CloseHandle(hComm);
#else
#endif		
	}
	return ret;
}
/*------------------------------------------------------------------------------------------------------*/
int com_handle_close(com_handle_st *) {
	int ret = 0;
	return ret;
}
/*------------------------------------------------------------------------------------------------------*/
int com_handle_get_cmd(com_handle_st *obj, int *cmd) {
	int ret = 0;
	do {
		if(cmd) {
			*cmd = obj->cmd;
		}
	} while(0);
	return ret;
}
/*------------------------------------------------------------------------------------------------------*/
int com_handle_set_cmd(com_handle_st *obj, int val) {
	int ret = 0;
	do {
		obj->cmd = val;
	} while(0);
	return ret;
}
/*------------------------------------------------------------------------------------------------------*/
int com_handle_read(com_handle_st *obj, int *cmd) {
	int ret = 0;
	return ret;
}
/*------------------------------------------------------------------------------------------------------*/
int com_handle_write(com_handle_st *obj, int *cmd) {
	int ret = 0;
	return ret;
}
/*------------------------------------------------------------------------------------------------------*/
void *read_write_com_routine(com_handle_st *obj) {
	void *ret = 0;
	int err = 0;
	int cmd = (int)COM_HANDLE_CMD_ENUM_OK;
	while(1) {
		err = com_handle_get_cmd(obj, &cmd);
		if(err) {
			break;
		}
		if(cmd == (int)COM_HANDLE_CMD_ENUM_STOP) {
			break;
		}
		ret = com_handle_open(obj);
		if(ret) {
			break;
		}
		while(1) {
			err = com_handle_get_cmd(obj, &cmd);
			if(err) {
				break;
			}
			if(cmd == (int)COM_HANDLE_CMD_ENUM_STOP) {
				break;
			}
			ret = com_handle_read(obj);
			if(ret) {
				break;
			}
			ret = com_handle_write(obj);
			if(ret) {
				break;
			}			
		}
	}
	ret = com_handle_close(obj);
	return ret;
}
/*------------------------------------------------------------------------------------------------------*/