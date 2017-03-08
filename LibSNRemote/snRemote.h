#ifndef _AMT_SN_H_
#define _AMT_SN_H_

enum SN_REMOTE_ERROR {
	REQUEST_SOCKET_FAILED			= -200,
	CONNECT_SOCKET_FAILED			= -201,
	SEND_DATA_FAILED				= -202,
	RECEIVE_DATA_FAILED				= -203,
	INVALID_MESSAGE_TYPE			= -204,
	GENERATE_SN_FAILED				= -205,
	READ_SERVER_FAILED				= -206,
	READ_USERID_FAILED				= -207,
	REMOTE_HOST_NOT_FOUND			= -208,
	NO_DATA_RECORD_FOUND			= -209,
	GET_HOST_BY_NAME_FAILED			= -210,
	NETBIOS_ADDRESS_RETURN			= -211,
};


bool isvalidSN(char * i_cpuid, char * i_sn);
int generateSN(char * i_cpuid, char * o_sn);

#endif