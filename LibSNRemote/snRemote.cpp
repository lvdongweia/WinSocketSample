#include "stdafx.h"
#include "snRemote.h"
#include <WinSock2.h>  
#include <stdio.h>  
#include <stdlib.h>
#include <malloc.h>
#pragma comment(lib, "WS2_32.lib")  

#define SN_PORT_NUM 0x9999
#define SN_SERVER_ADDR_LEN  100
#define BUFFER_SIZE  1024

#define SNLEN     18
#define CPUIDLEN  12
#define IPLEN  15
#define UIDLEN 30

enum SNC_MSG_TYPE {
        SN_GENERATE_REQUEST,
        SN_GENERATE_RESPONSE,
        SN_VALIDATE_REQUEST,
        SN_VALIDATE_RESPONSE,
        MSG_TYPE_MAX
};

struct snc_msg_header {
        char type;
        char len;
};

#define SNC_MSG_RESPONSE  \
        char ret;  \
        char error_code;

struct snc_msg_common_response {
        SNC_MSG_RESPONSE
};

/* sn generate message */
struct snc_msg_generate_request {
        char uid[UIDLEN + 1];
        char uip[IPLEN + 1];
        char cpuid[CPUIDLEN + 1];
};

struct snc_msg_generate_response {
        SNC_MSG_RESPONSE
        char sn[SNLEN + 1];
};

/* sn validate message */
struct snc_msg_validate_request {
        char uid[UIDLEN + 1];
        char uip[IPLEN + 1];
        char cpuid[CPUIDLEN + 1];
        char sn[SNLEN + 1];
};

struct snc_msg_validate_response {
        SNC_MSG_RESPONSE
        char validate_result; /* 0: success -1:fail */
};


struct snc_msg {
        struct snc_msg_header header;
        union {
                struct snc_msg_generate_request sn_generate_request;
                struct snc_msg_generate_response sn_generate_response;
                struct snc_msg_validate_request sn_validate_request;
                struct snc_msg_validate_response sn_validate_response;
        }u;
};

int getIpByDomainName(char *i_name, char *o_ip)
{
    struct hostent *remoteHost;
	DWORD dwError;
	int i = 0;
	char **pAlias;
	struct in_addr addr;

	remoteHost = gethostbyname(i_name);

    if (remoteHost == NULL) {
        dwError = WSAGetLastError();
        if (dwError != 0) {
            if (dwError == WSAHOST_NOT_FOUND) {
                printf("Host not found\n");
                return REMOTE_HOST_NOT_FOUND;
            } else if (dwError == WSANO_DATA) {
                printf("No data record found\n");
                return NO_DATA_RECORD_FOUND;
            } else {
                printf("Function failed with error: %ld\n", dwError);
                return GET_HOST_BY_NAME_FAILED;
            }
        }
    } else {
        printf("Function returned:\n");
        printf("\tOfficial name: %s\n", remoteHost->h_name);
        for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
            printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
        }
        printf("\tAddress type: ");
        switch (remoteHost->h_addrtype) {
        case AF_INET:
            printf("AF_INET\n");
            break;
        case AF_NETBIOS:
            printf("AF_NETBIOS\n");
            break;
        default:
            printf(" %d\n", remoteHost->h_addrtype);
            break;
        }
        printf("\tAddress length: %d\n", remoteHost->h_length);

        i = 0;
        if (remoteHost->h_addrtype == AF_INET)
        {
            while (remoteHost->h_addr_list[i] != 0) {
                addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
                printf("\tIP Address #%d: %s\n", i, inet_ntoa(addr));
				sprintf(o_ip, "%s", inet_ntoa(addr));
				return 0;
            }
        }
        else if (remoteHost->h_addrtype == AF_NETBIOS)
        {   
            printf("NETBIOS address was returned\n");
			return NETBIOS_ADDRESS_RETURN;
        }   
    }

	return 0;
}

bool isValidIP(char *ip){
    if(ip==NULL)
        return false;
    char temp[4];
    int count=0;
    while(true){
        int index=0;
        while(*ip!='\0' && *ip!='.' && count<4){
			if(*ip < '0' || *ip > '9') return false;
            temp[index++]=*ip;
            ip++;
        }
        if(index==4)
            return false;
        temp[index]='\0';
        int num=atoi(temp);
        if(!(num>=0 && num<=255))
            return false;
        count++;
        if(*ip=='\0'){
            if(count==4)
                return true;
            else
                return false;
        }else
            ip++;
    }
}

int getServerIp(char *serverip)
{
	int ret;

	/*Get the path*/
	WCHAR wPath[MAX_PATH] = {0};
	GetModuleFileName(NULL,wPath,MAX_PATH);   

	WCHAR wConfigFile[MAX_PATH] = L"\\config.ini";
	WCHAR wConfigPathFile[MAX_PATH] = {0};
	int   nPos;
	size_t pathlen = wcslen(wPath) + 1;
	nPos = pathlen;
	while(wPath[nPos] != '\\' && nPos > 0) nPos--;
	wcsncpy_s(wConfigPathFile, wPath, nPos);
	wcscpy_s(&wConfigPathFile[nPos], wcslen(wConfigFile) + 1, wConfigFile);

	/*Read the server*/
	WCHAR wServer[SN_SERVER_ADDR_LEN + 1];
    GetPrivateProfileString(L"libSettingsInfo",L"snserver",L"",wServer,IPLEN,wConfigPathFile);
	if (wServer[0] == '\0')
		return READ_SERVER_FAILED;

    /*Convert the wchar into char for server*/
	size_t len = wcslen(wServer) + 1;	
	size_t converted = 0;
	char *cServer;
	cServer=(char*)malloc(len*sizeof(char));
	wcstombs_s(&converted, cServer, len, wServer, _TRUNCATE);	

	/*Check if the server address is IP address*/	
	if (isValidIP(cServer))
	{
		strncpy(serverip, cServer, IPLEN);
		ret = 0;
	}
	else
	{
		ret = getIpByDomainName(cServer, serverip);
		if (ret <0)
		{
			OutputDebugString(L"get server ip failed\n");
		}
	}

	free(cServer);
	return ret;
}

int getUserID(char *uid)
{
	/*Get the path*/
	WCHAR wPath[MAX_PATH] = {0};
	GetModuleFileName(NULL,wPath,MAX_PATH);   

	WCHAR wConfigFile[MAX_PATH] = L"\\config.ini";
	WCHAR wConfigPathFile[MAX_PATH] = {0};
	int   nPos;
	size_t pathlen = wcslen(wPath) + 1;
	nPos = pathlen;
	while(wPath[nPos] != '\\' && nPos > 0) nPos--;
	wcsncpy_s(wConfigPathFile, wPath, nPos);
	wcscpy_s(&wConfigPathFile[nPos], wcslen(wConfigFile) + 1, wConfigFile);

	/*Read the userid*/
	WCHAR wUserID[UIDLEN + 1];
    GetPrivateProfileString(L"libSettingsInfo",L"userid",L"",wUserID,UIDLEN,wConfigPathFile);
	if (wUserID[0] == '\0')
		return READ_USERID_FAILED;

    /*Convert the wchar into char for userid*/
	size_t len = wcslen(wUserID) + 1;
	size_t converted = 0;
	char *cServerIp;
	cServerIp=(char*)malloc(len*sizeof(char));
	wcstombs_s(&converted, cServerIp, len, wUserID, _TRUNCATE);	
	strncpy(uid, cServerIp, UIDLEN);
	free(cServerIp);

	return 0;
}

int getHostName(char *hostname)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    gethostname(hostname, 128);
    WSACleanup();
    return 0;
}

int getHostIP(char *hostip)
{
	char host_name[128] = "";
	getHostName(host_name);
    int ret = getIpByDomainName(host_name, hostip);
	if (ret <0)
	{
		OutputDebugString(L"get host ip failed\n");
		return ret;
	}

	return 0;
}

int connectServer(SOCKET *psock)
{
	WSADATA wsaData;  
    WORD sockVersion = MAKEWORD(2,0);//指定版本号  
    ::WSAStartup(sockVersion, &wsaData);//载入winsock的dll  
    //建立socket，基于tcp  
    SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    if(s == INVALID_SOCKET)  
    {  
        OutputDebugString(L"error\n");  
        ::WSACleanup(); //释放资源  
        return REQUEST_SOCKET_FAILED;  
    }  
  
	char serverip[IPLEN + 1] = {0};
	int ret = getServerIp(serverip);
	if (ret < 0)
	{
		OutputDebugString(L"get Server IP failed\n");  
        ::WSACleanup(); //释放资源  
        return ret;		
	}

    sockaddr_in servAddr;  
    servAddr.sin_family = AF_INET;  
    servAddr.sin_port = htons(SN_PORT_NUM);//端口号  
    servAddr.sin_addr.S_un.S_addr = inet_addr(serverip);//IP  

    //连接  
    if(::connect(s, (sockaddr*)&servAddr, sizeof(servAddr)) == -1)  
    {  
        OutputDebugString(L"error\n");  
        ::WSACleanup(); //释放资源  
        return CONNECT_SOCKET_FAILED;  
    }  
	*psock = s;
	return 0;
}

int disconnectServer(SOCKET sock)
{
	::closesocket(sock); //关闭套接字 
	::WSACleanup(); //释放资源  
	return 0;
}

int sendData(SOCKET sock, char *buff, int len)
{
	int nSend = ::send(sock, buff, len, 0); //发送数据 	
	if (nSend < 0)
	{
		OutputDebugString(L"发送数据失败\n"); 
		return SEND_DATA_FAILED;
	}
	else
	{
        buff[nSend] = '\0';  
        OutputDebugString(L"发送数据成功\n");
		return 0;
	}
}


int recvData(SOCKET sock, char *buff, int len)  
{  
    int nRecv = ::recv(sock, buff, len, 0);//接收数据  
    if(nRecv > 0)  
    {  
        buff[nRecv] = '\0';  
        OutputDebugString(L"接受数据成功\n");  
		return 0;
    }
	else
	{
		OutputDebugString(L"接受数据失败\n"); 
		return RECEIVE_DATA_FAILED;
	}
}  

bool isvalidSN(char * i_cpuid, char * i_sn)
{
	return true;
}

int generateSN(char * i_cpuid, char * o_sn)
{
	SOCKET sock;
	struct snc_msg *psnm_send = NULL;
	int ret = 0, len;
	char buffer[BUFFER_SIZE]={0};
	struct snc_msg *psnm_recv = NULL;

	ret = connectServer(&sock);
	if (ret < 0)
	{
		goto connectServerFailed;
	}

	char uid[UIDLEN + 1];
	ret = getUserID(uid);
	if (ret < 0)
	{
		OutputDebugString(L"get User ID failed\n");  
        goto out;		
	}

	char hostip[IPLEN + 1] = {0};
	ret = getHostIP(hostip);
	if (ret < 0)
	{
		OutputDebugString(L"get Host IP failed\n");  
        goto out;		
	}

	len = sizeof(struct snc_msg);
	psnm_send = (struct snc_msg *)malloc(len);
	memset(psnm_send, 0, len);

	psnm_send->header.type = SN_GENERATE_REQUEST;
	memcpy(psnm_send->u.sn_generate_request.uid, uid, UIDLEN);
	memcpy(psnm_send->u.sn_generate_request.uip, hostip, IPLEN);
	memcpy(psnm_send->u.sn_generate_request.cpuid, i_cpuid, CPUIDLEN);
	psnm_send->header.len = len;
	
	ret = sendData(sock, (char *) psnm_send, psnm_send->header.len - 1); //awhuo, if not less 1, heap corruption detected 
	if(ret < 0)
	{
		OutputDebugString(L"send data fail!\r\n");
		goto out;
	}

	ret = recvData(sock, buffer, BUFFER_SIZE);
	if(ret < 0)
	{
		OutputDebugString(L"receive data fail !\r\n");
		goto out;
	}
	psnm_recv = (struct snc_msg *) buffer;

	if (psnm_recv->header.type < 0 || psnm_recv->header.type >= MSG_TYPE_MAX ||
		psnm_recv->header.type != SN_GENERATE_RESPONSE)
	{
		OutputDebugString(L"Invalid message type !\r\n");
		ret = INVALID_MESSAGE_TYPE;
		goto out;
	}
	if (psnm_recv->u.sn_generate_response.ret < 0)
	{
		OutputDebugString(L"Generate sn failed !\r\n");
		ret = GENERATE_SN_FAILED;
		goto out;
	}
	memcpy(o_sn, psnm_recv->u.sn_generate_response.sn, SNLEN);
out:
	if (psnm_send) free(psnm_send);
	disconnectServer(sock);

connectServerFailed:
	return ret;
}