// PTBase.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include "ControlCAN.h"

using namespace std;

#define DLL_IMPLEMENT
#include "PTBase.h"
#include "debug.h"

#define PACKET_MAX_NUM             256
#define CAN_DATA_MAX_LEN           8
#define CAN_SUB_PACKET_OFFSET      2       /* 0: id; 1: SN */

const uint8_t TimeOutId = 127;
enum CAN_CHANNEL canind = CHANNEL_ONE;

/*
 * Note: if packet will send and is incomplete,
 * @len <= CAN_DATA_MAX_LEN - 2
 * data[0] = packet_id;
 * data[1] = packet_no;
 *
 * .sum == 0:
 * packet is complete.
 * .sum > 0:
 * packet is incomplete.
 */
struct flexcan_packet
{
    uint8_t packet_id;             /* uniquely identifies a entry */
    uint8_t priority;
    uint8_t s_id;
    uint8_t d_id;
    uint8_t data[CAN_DATA_MAX_LEN];
    uint32_t len;                  /* real data len <= CAN_DATA_MAX_LEN */
    uint8_t packet_SN;             /* packets number */
    uint8_t sum;                   /* starting from 0 */
};

struct CANPacket
{
    uint8_t  priority;
    uint8_t  src_id;
    uint8_t  dest_id;
    void     *p_data;
    uint32_t len;

    CANPacket() { p_data = NULL; }
    ~CANPacket() { if (p_data) free(p_data); }
};

typedef std::vector<struct flexcan_packet> UDT_VEC_FLEXCAN;
typedef std::map<uint8_t,  UDT_VEC_FLEXCAN> UDT_MAP_INT_FLEXCAN;
typedef std::map<uint8_t,  UDT_MAP_INT_FLEXCAN> UDT_MAP_INT_INT_FLEXCAN;
typedef std::list<struct CANPacket*> UDT_LIST_CANPACKET;
typedef std::list<VCI_CAN_OBJ> UDT_LIST_CAN_OBJ;

static uint8_t PacketsId = 0;
/* <src_id, <packet_id, packets>> */
UDT_MAP_INT_INT_FLEXCAN InCompletePackets;
UDT_LIST_CANPACKET RecvList;
static HANDLE RecvLock = INVALID_HANDLE_VALUE;   /* protection recv list */
UDT_LIST_CAN_OBJ CanObjList;
static HANDLE CanObjLock = INVALID_HANDLE_VALUE;   /* protection CanObj output */

/* Micro */
#define CAN_SUB_PACKET_SIZE        (CAN_DATA_MAX_LEN - CAN_SUB_PACKET_OFFSET)
#define CAN_DATA_TOTAL_SIZE        (CAN_SUB_PACKET_SIZE * PACKET_MAX_NUM)


/* send */
#define CAN_DATA2ID_PR_MASK     0x7   /* priority   */
#define CAN_DATA2ID_SYS_MASK    0xF0  /* system id  */
#define CAN_DATA2ID_SMOD_MASK   0x0F  /* module id  */
#define CAN_DATA2ID_SUM_MASK    0xFF  /* packet sum */

#define CAN_D_SMOD_OFFSET       0
#define CAN_S_SMOD_OFFSET       4
#define CAN_SUM_OFFSET          8
#define CAN_D_SYS_OFFSET        14
#define CAN_S_SYS_OFFSET        18
#define CAN_PRIORITY_OFFSET     26

#define CAN_DATA2ID_PRIORITY(p) ((uint32_t)((p) & CAN_DATA2ID_PR_MASK) << CAN_PRIORITY_OFFSET)
#define CAN_DATA2ID_S_SYS(s)    ((uint32_t)((s) & CAN_DATA2ID_SYS_MASK) << CAN_S_SYS_OFFSET)
#define CAN_DATA2ID_D_SYS(d)    ((uint32_t)((d) & CAN_DATA2ID_SYS_MASK) << CAN_D_SYS_OFFSET)
#define CAN_DATA2ID_S_SMOD(s)   ((uint32_t)((s) & CAN_DATA2ID_SMOD_MASK) << CAN_S_SMOD_OFFSET)
#define CAN_DATA2ID_D_SMOD(d)   ((uint32_t)((d) & CAN_DATA2ID_SMOD_MASK))
#define CAN_DATA2ID_SUM(sum)    ((uint32_t)((sum) & CAN_DATA2ID_SUM_MASK) << CAN_SUM_OFFSET)

/* recv */
#define CAN_ID2DATA_PR_MASK     0x1C000000
#define CAN_ID2DATA_S_SYS_MASK  0x03C00000
#define CAN_ID2DATA_D_SYS_MASK  0x003C0000
#define CAN_ID2DATA_S_SMOD_MASK 0x000000F0
#define CAN_ID2DATA_D_SMOD_MASK 0x0000000F
#define CAN_ID2DATA_SUM_MASK    0x0000FF00

#define CAN_ID2DATA_PRIORITY(p) (uint8_t)(((p) & CAN_ID2DATA_PR_MASK) >> CAN_PRIORITY_OFFSET)
#define CAN_ID2DATA_S_SYS(s)    (uint8_t)(((s) & CAN_ID2DATA_S_SYS_MASK) >> CAN_S_SYS_OFFSET)
#define CAN_ID2DATA_D_SYS(d)    (uint8_t)(((d) & CAN_ID2DATA_D_SYS_MASK) >> CAN_D_SYS_OFFSET)
#define CAN_ID2DATA_S_SMOD(s)   (uint8_t)(((s) & CAN_ID2DATA_S_SMOD_MASK) >> CAN_S_SMOD_OFFSET)
#define CAN_ID2DATA_D_SMOD(d)   (uint8_t)(((d) & CAN_ID2DATA_D_SMOD_MASK))
#define CAN_ID2DATA_SUM(m)      (uint8_t)(((m) & CAN_ID2DATA_SUM_MASK) >> CAN_SUM_OFFSET)
#define CAN_ID2DATA_S_MODULE(s) (CAN_ID2DATA_S_SYS(s) | CAN_ID2DATA_S_SMOD(s))
#define CAN_ID2DATA_D_MODULE(d) (CAN_ID2DATA_D_SYS(d) | CAN_ID2DATA_D_SMOD(d))

void InitPTBase()
{
	CanObjLock = CreateMutex(NULL, FALSE, L"Mutex"); 
	RecvLock = CreateMutex(NULL, FALSE, L"Mutex"); 

	WaitForSingleObject(CanObjLock, INFINITE );
	CanObjList.clear();
	ReleaseMutex(CanObjLock);

	WaitForSingleObject(RecvLock, INFINITE );
	RecvList.clear();
	ReleaseMutex(RecvLock);

	InCompletePackets.clear();
}

void UninitPTBase()
{
	WaitForSingleObject(CanObjLock, INFINITE );
	CanObjList.clear();
	ReleaseMutex(CanObjLock);

	WaitForSingleObject(RecvLock, INFINITE );
	RecvList.clear();
	ReleaseMutex(RecvLock);

	InCompletePackets.clear();
}

//打开设备
int OpenDevice() 
{
	int DevIndex=0;	
	int DevType = VCI_USBCAN2;
	DWORD Reserved=0;

	//打开设备
	if(VCI_OpenDevice(DevType,DevIndex,Reserved)!=1)
	{
		return OPEN_DEVICE_FAILED;
	}
	VCI_INIT_CONFIG InitInfo[1];
	InitInfo->Timing0=0x00;
	InitInfo->Timing1=0x1C;//500kbps
	InitInfo->Filter=0;
	InitInfo->AccCode=0x80000000;
	InitInfo->AccMask=0xFFFFFFFF;
	InitInfo->Mode=0;
	//InitInfo->Mode=2;//自发自收

	//初始化通道0
	if(VCI_InitCAN(DevType,DevIndex, 0,InitInfo)!=1)	//can-0
	{
		return INIT_CAN_FAILED;
	}
	Sleep(100);
    //初始化通道0
	if(VCI_StartCAN(DevType,DevIndex, 0)!=1)	//can-0
	{
		return START_CAN_FAILED;
	}
    //初始化通道1
	if(VCI_InitCAN(DevType,DevIndex, 1,InitInfo)!=1)	//can-1
	{
		return INIT_CAN_FAILED;
	}
	Sleep(100);
    //初始化通道1
	if(VCI_StartCAN(DevType,DevIndex, 1)!=1)	//can-0
	{
		return START_CAN_FAILED;
	}
	
	return NO_PT_ERROR;
}

//关闭设备
int CloseDevice() 
{
	int DevIndex=0;	
	int DevType = VCI_USBCAN2;

	if(VCI_CloseDevice(DevType,DevIndex)!=1)
	{
		return CLOSE_DEVICE_FAILED;		
	}
	return NO_PT_ERROR;
}

static void dump_vci_can_obj(VCI_CAN_OBJ *pCanObj)
{
	PTDbgStr("  ID:0x%x\n", pCanObj->ID);
	PTDbgStr("  TimeStamp:%d\n", pCanObj->TimeStamp);
	PTDbgStr("  TimeFlag:%d\n", pCanObj->TimeFlag);
	PTDbgStr("  SendType:%d\n", pCanObj->SendType);
	PTDbgStr("  RemoteFlag:%d\n", pCanObj->RemoteFlag);
	PTDbgStr("  ExternFlag:%d\n", pCanObj->ExternFlag);
	PTDbgStr("  DataLen:%d\n", pCanObj->DataLen);
	for (int i = 0; i < pCanObj->DataLen; i++)
		PTDbgStr("  %02x", pCanObj->Data[i]);
	PTDbgStr("\n");
}

static void dump_flexcan_packet(struct flexcan_packet *pcan_data)
{
	PTDbgStr("  packet_id:%d\n", pcan_data->packet_id);
	PTDbgStr("  priority:%d\n", pcan_data->priority);
	PTDbgStr("  s_id:%02x\n", pcan_data->s_id);
	PTDbgStr("  d_id:%02x\n", pcan_data->d_id);
	PTDbgStr("  len:%d\n", pcan_data->len);
	for (uint32_t i = 0; i < pcan_data->len; i++)
		PTDbgStr("  %02x", pcan_data->data[i]);
	PTDbgStr("\n");
	PTDbgStr("  packet_SN:%d\n", pcan_data->packet_SN);
	PTDbgStr("  sum:%d\n", pcan_data->sum);
}

static int _flexcan_send(const struct flexcan_packet *packet)
{
	uint32_t i;
	int DevIndex=0;	
	int DevType = VCI_USBCAN2;

    if (!packet || packet->len <= 0 || packet->len > CAN_DATA_MAX_LEN)
    {
        return INVALID_PARAMETER;
    }

	PTDbgStr("Send flexcan_packet:\n");
	dump_flexcan_packet((struct flexcan_packet *)packet);

	VCI_CAN_OBJ sendbuf[1];
	sendbuf->ExternFlag=1;
	sendbuf->RemoteFlag=0;
	sendbuf->ID = CAN_DATA2ID_PRIORITY(packet->priority)
        | CAN_DATA2ID_S_SYS(packet->s_id)
        | CAN_DATA2ID_D_SYS(packet->d_id)
        | CAN_DATA2ID_S_SMOD(packet->s_id)
        | CAN_DATA2ID_D_SMOD(packet->d_id);

	    /* complete packet */
    if (packet->sum == 0)
    {
		for(i=0;i<packet->len;i++)
			sendbuf->Data[i]=packet->data[i];

		sendbuf->DataLen=packet->len;

		PTDbgStr("Send pCanObj:\n");
		dump_vci_can_obj(sendbuf);

		int flag;
		int	nCanIndex=canind;

		//调用动态链接库发送函数
		flag=VCI_Transmit(DevType,DevIndex,nCanIndex,sendbuf,1);//CAN message send
		if(flag<1)
		{
			if(flag==-1)
				return DEVICE_NOT_OPEN;
			else if(flag==0)			
				return SEND_ERROR;
	
		}
    }
    else
    {
        /* incomplete packet */
        if (packet->len > CAN_SUB_PACKET_SIZE)
        {
            return PACKET_IS_INCOMPLETE;
        }

        /* starting from 0(sum + 1)*/
        sendbuf->ID |= CAN_DATA2ID_SUM(packet->sum);

		memset(sendbuf->Data, 0, sizeof(sendbuf->Data));
        sendbuf->Data[0] = packet->packet_id;
        sendbuf->Data[1] = packet->packet_SN;

		for(i=0;i<packet->len;i++)
			sendbuf->Data[CAN_SUB_PACKET_OFFSET + i]=packet->data[i];

		sendbuf->DataLen=packet->len + CAN_SUB_PACKET_OFFSET;

		PTDbgStr("Send pCanObj:\n");
		dump_vci_can_obj(sendbuf);

		int flag;
		int	nCanIndex=0;

		//调用动态链接库发送函数
		flag=VCI_Transmit(DevType,DevIndex,nCanIndex,sendbuf,1);//CAN message send
		if(flag<1)
		{
			if(flag==-1)
				return DEVICE_NOT_OPEN;
			else if(flag==0)			
				return SEND_ERROR;
	
		}
    }
	return NO_PT_ERROR;
}

int CANSend(const struct flexcan_packet *packet)
{
    int ret = _flexcan_send(packet);
	int times = 3;
    while (ret < 0)
    {
		if (times < 0) break;
        ret = _flexcan_send(packet);
        Sleep(3);   //sleep 3ms, again send
		times--;
    }

    return ret;
}


//发送信息
int SendCanData(int priority, int src_id, int dest_id,
        const void *pdata, int len)
{
	int ret = -1;
    struct flexcan_packet can_data;
    memset(&can_data, 0, sizeof(struct flexcan_packet));

    /* common data */
    can_data.priority = priority;
    can_data.s_id     = src_id;
    can_data.d_id     = dest_id;

	if (len <= CAN_DATA_MAX_LEN)
    {
        can_data.len  = len;
        memcpy(can_data.data, pdata, len);
        ret = CANSend(&can_data);

        /*
         * memset:
         * can_data.packet_id = 0;
         * can_data.packet_SN = 0;
         * can_data.sum       = 0;
         */
    }
    else
    {
        can_data.packet_id = PacketsId;
        PacketsId++;

        uint8_t num = (len % CAN_SUB_PACKET_SIZE == 0)
            ? (len / CAN_SUB_PACKET_SIZE)
            : (len / CAN_SUB_PACKET_SIZE + 1);
        can_data.sum = num - 1;                                /* 0 ~ num - 1 */

        uint8_t *tmp_p = (uint8_t*)(pdata);
        can_data.len = CAN_SUB_PACKET_SIZE;
        /* 0 ~ num-2 */
        for(int i = 0; i < num - 1; ++i)
        {
            can_data.packet_SN = i;
            memcpy(can_data.data, tmp_p, CAN_SUB_PACKET_SIZE);

            ret = CANSend(&can_data);
            if (ret != 0) goto send_error;

            tmp_p += CAN_SUB_PACKET_SIZE;
            memset(can_data.data, 0, CAN_DATA_MAX_LEN);
			//Sleep(10);
        }

        /* send lastest sub-packet */
        uint32_t nleft = len - (num - 1) * CAN_SUB_PACKET_SIZE;
        can_data.len = nleft;
        can_data.packet_SN = num - 1;
        memcpy(can_data.data, tmp_p, nleft);

        ret = CANSend(&can_data);
		//Sleep(10);
    }

send_error:
    return ret;
}



void PushToCanObjList(VCI_CAN_OBJ can_obj)
{
	WaitForSingleObject(CanObjLock, INFINITE );
	CanObjList.push_front(can_obj);
	ReleaseMutex(CanObjLock);
}

bool isCanObjListEmpty()
{
	bool isEmpty;
	WaitForSingleObject(CanObjLock, INFINITE );
	isEmpty = CanObjList.empty();
	ReleaseMutex(CanObjLock);
	return isEmpty;
}

void PopFromCanObjList(VCI_CAN_OBJ *can_obj_ptr)
{
	VCI_CAN_OBJ can_obj_local;
	UDT_LIST_CAN_OBJ::iterator i;
	WaitForSingleObject(CanObjLock, INFINITE );
	if (!CanObjList.empty())
	{
		i = CanObjList.end();
		can_obj_local = *(--i);
		*can_obj_ptr = can_obj_local;
		CanObjList.erase(i);
	}
	ReleaseMutex(CanObjLock);
}

static void can_obj_recv(int kCanIndex)
{
	int DevIndex=0;	
	int DevType = VCI_USBCAN2;
	int NumValue;
	VCI_CAN_OBJ pCanObj[200];
	int num=0;

	//调用动态链接看接收函数
	NumValue=VCI_Receive(DevType,DevIndex,kCanIndex,pCanObj,200,0);
	for(num=0;num<NumValue;num++)
	{
		PushToCanObjList(pCanObj[num]);
	}
}

volatile BOOL bCanObjRun=TRUE;
HANDLE hCanObjThread;
DWORD CanObjThreadID;

void CanObjThreadFunc()
{
	while(bCanObjRun)
	{
		for(int kCanIndex=0;kCanIndex<2;kCanIndex++)
		{
			can_obj_recv(kCanIndex);
		}
		Sleep(1);
	}
}

void StartCanObj()
{
	bCanObjRun=TRUE;
	// TODO: Add your control notification handler code here
	hCanObjThread=CreateThread(NULL,
	0,
	(LPTHREAD_START_ROUTINE)CanObjThreadFunc,
	NULL,
	0,
	&CanObjThreadID);
}
void StopCanObj()
{
	bCanObjRun=FALSE;
}


static int _flexcan_recv(struct flexcan_packet *packet, int kCanIndex)
{
    if (!packet)
    {
        return INVALID_PARAMETER;
    }

	VCI_CAN_OBJ pCanObj[1];

	if (isCanObjListEmpty())
	{
		return NO_CAN_OBJ;
	}

	PopFromCanObjList(pCanObj);

	PTDbgStr("Receive pCanObj:\n");
	dump_vci_can_obj(pCanObj);


	uint32_t extid = pCanObj->ID;

    /* parse frame id */
    packet->priority = CAN_ID2DATA_PRIORITY(extid); /* priority       */
    packet->s_id     = CAN_ID2DATA_S_MODULE(extid); /* src module id  */
    packet->d_id = CAN_ID2DATA_D_MODULE(extid);

    uint32_t sum = CAN_ID2DATA_SUM(extid);
    packet->sum = sum;

    /* set 0 in external */
    //memset(packet->data, 0, CAN_DATA_MAX_LEN);

    /* just one can packet */
    if (0 == sum)
    {
		memcpy(packet->data, pCanObj->Data, pCanObj->DataLen);

		packet->len = pCanObj->DataLen;
        packet->packet_id = 0;
        packet->packet_SN = 0;
        return NO_PT_ERROR;
    }

    /* more packets, incomplete packet */
    memcpy(packet->data, pCanObj->Data + CAN_SUB_PACKET_OFFSET,
            pCanObj->DataLen - CAN_SUB_PACKET_OFFSET);

    packet->len = pCanObj->DataLen - CAN_SUB_PACKET_OFFSET;
    packet->packet_id = pCanObj->Data[0];
    packet->packet_SN = pCanObj->Data[1];

    return NO_PT_ERROR;
}

int CANRecv(struct flexcan_packet *packet, int kCanIndex)
{
    /* set 0 in external */
    //memset(packet, 0, sizeof(struct flexcan_packet));

    return _flexcan_recv(packet, kCanIndex);
}


void RemovePacketId(UDT_MAP_INT_FLEXCAN &packets, UDT_MAP_INT_FLEXCAN::iterator itt)
{
	itt->second.clear();
	packets.erase(itt);
}

static int PacketCompare(
        const struct flexcan_packet &lpacket,
        const struct flexcan_packet &rpacket)
{
    return (lpacket.packet_SN < rpacket.packet_SN);
}

CANPacket * GetCANPacket(
        int priority, int src_id, int dest_id,
        const void *pdata, int len)
{
    CANPacket *packet = NULL;

    void *ptr = malloc(len);
    if (!ptr)
    {
        PTDbgStr("malloc request failed");
        return NULL;
    }

    memcpy(ptr, pdata, len);

    packet = new CANPacket;
    packet->priority = priority;
    packet->src_id   = src_id;
    packet->dest_id  = dest_id;
    packet->p_data   = ptr;
    packet->len      = len;

    return packet;
}

/* constructor packet from can_data */
CANPacket* GetCANPacket(UDT_VEC_FLEXCAN packets)
{
	sort(packets.begin(), packets.end(), PacketCompare);

    uint8_t data[CAN_DATA_TOTAL_SIZE];
    uint32_t len = 0;

    memset(data, 0, CAN_DATA_TOTAL_SIZE);

    vector<struct flexcan_packet>::iterator iter;
    for (iter = packets.begin(); iter != packets.end(); iter++)
    {
        memcpy(data + len, (*iter).data, (*iter).len);
        len += (*iter).len;
    }

    CANPacket *packet = GetCANPacket(
            packets.at(0).priority, packets.at(0).s_id,
            packets.at(0).d_id, data, len);

    return packet;
}

CANPacket *GetCANPacket(const struct flexcan_packet *can_data)
{
    CANPacket *packet = GetCANPacket(
            can_data->priority, can_data->s_id,
            can_data->d_id, can_data->data,
            can_data->len);

    return packet;
}

void PushToRecvList(CANPacket *packet)
{
	WaitForSingleObject(RecvLock, INFINITE );
	RecvList.push_front(packet);
	ReleaseMutex(RecvLock);
}

bool isRecvListEmpty()
{
	bool isEmpty;
	WaitForSingleObject(RecvLock, INFINITE );
	isEmpty = RecvList.empty();
	ReleaseMutex(RecvLock);
	return isEmpty;
}

void PopFromRecvList(CANPacket **packet)
{
	CANPacket *packet_local = NULL;
	UDT_LIST_CANPACKET::iterator i;
	WaitForSingleObject(RecvLock, INFINITE );
	if (!RecvList.empty())
	{
		i = RecvList.end();
		packet_local = *(--i);
		*packet = GetCANPacket(
			packet_local->priority, packet_local->src_id,
			packet_local->dest_id, packet_local->p_data,
			packet_local->len);
		delete packet_local;
		RecvList.erase(i);
	}
	ReleaseMutex(RecvLock);
}

/* recv can data from can driver */
int RecvFromCAN(int kCanIndex)
{
    struct flexcan_packet can_data;
    memset(&can_data, 0, sizeof(struct flexcan_packet));

    int ret = CANRecv(&can_data, kCanIndex);
    if (ret < 0) return ret;

	PTDbgStr("Receive flexcan_packet:\n");
	dump_flexcan_packet(&can_data);

    CANPacket *packet = NULL;

    /* incomplete packet */
    if (can_data.sum != 0)
    {
        /* need to construct a packet */
        /* <src_id, KeyVector> */
		UDT_MAP_INT_INT_FLEXCAN::iterator it = InCompletePackets.find(can_data.s_id);
		if (it != InCompletePackets.end())
        {
            /* <packet_id, Vector> */
			UDT_MAP_INT_FLEXCAN::iterator itt = it->second.find(can_data.packet_id);
			if (itt != it->second.end())
            {                
				itt->second.push_back(can_data);
				size_t num = itt->second.size();
                /* sum: 0 ~ num-1 */
                //TODO: packet error handle ??????????????????
				PTDbgStr("can_data.sum:%d, list num:%d\n", can_data.sum, num);
                if ((can_data.sum + 1) <= (uint8_t)num)
                {
                    packet =
						GetCANPacket(itt->second);

					RemovePacketId(it->second, itt);
                    PTDbgStr("complete packets num = %u\n", num);
                    goto complete;
                }
            }
            else
            {
                /* delete old packet_id(timeout id)
                 * Note: Not a good solution, May lose data
                 */
                uint8_t old_packet_id = can_data.packet_id + TimeOutId;
				UDT_MAP_INT_FLEXCAN::iterator itt_old = it->second.find(old_packet_id);
                if (itt_old != it->second.end())
                {
                    RemovePacketId(it->second, itt_old);
                }

                /* add new packet_id */
                UDT_VEC_FLEXCAN packets;
                packets.push_back(can_data);

                /* KeyVector<packet_id, Vector> */
				it->second.insert(UDT_MAP_INT_FLEXCAN :: value_type(can_data.packet_id, packets));
            }
        }
        else
        {
            /* Vector<> */
            UDT_VEC_FLEXCAN packets;
            packets.push_back(can_data);

            /* KeyVector<packet_id, Vector> */
            UDT_MAP_INT_FLEXCAN packets_id;
			packets_id.insert(UDT_MAP_INT_FLEXCAN :: value_type(can_data.packet_id, packets));

            /* KeyVector<src_id, KeyVector> */
            InCompletePackets.insert(UDT_MAP_INT_INT_FLEXCAN :: value_type(can_data.s_id, packets_id));			
        }

        return 0;
    }

    /* complete packet */
    packet = GetCANPacket(&can_data);

complete:
    if (packet != NULL)
    {
        PushToRecvList(packet);
    }

    return 0;
}

volatile BOOL bRun=TRUE;
HANDLE hThread;
DWORD ThreadID;

void ThreadFunc()
{
	while(bRun)
	{
		for(int kCanIndex=0;kCanIndex<2;kCanIndex++)
		{
			RecvFromCAN(kCanIndex);
		}
		Sleep(1);
	}
}

 void StartRecvFromCAN()
{
	bRun=TRUE;
	// TODO: Add your control notification handler code here
	hThread=CreateThread(NULL,
	0,
	(LPTHREAD_START_ROUTINE)ThreadFunc,
	NULL,
	0,
	&ThreadID);
}

void StopRecvFromCAN()
{
	bRun=FALSE;
}

//接收信息
int ReceiveCanData(uint8_t expect_src_id, uint8_t expect_dest_id,
	uint8_t expect_msg_len, uint8_t expect_msg_type, 
	void *o_pdata, int tolerable_waiting_time,
	IS_CANCEL_CALLBACK is_cancel_func,
	void *canel_parameter)
{
	int timeout = tolerable_waiting_time;
	bool is_canceled = false;

	while (timeout-- >= 0)
	{
		CANPacket *packet = NULL;

		if (is_cancel_func)
		{
			if (is_cancel_func(canel_parameter))
			{
				is_canceled = true;
				break;
			}
		}
		if (!isRecvListEmpty())
			PopFromRecvList(&packet);

		if (packet)
		{
			PTDbgStr("priority:%d, src_id:%02x, dest_id:%02x, p_data:%d, len:%d\n", 
				packet->priority, packet->src_id, packet->dest_id, packet->p_data, packet->len);
			if (packet->src_id != expect_src_id)
			{
				PTDbgStr("It is not expect src id, expect:%02x, real:%02x!\n",
					expect_src_id, packet->src_id);
				delete packet;
				continue;
			}
			if (packet->dest_id != expect_dest_id)
			{
				PTDbgStr("It is not expect dest id, expect:%02x, real:%02x!\n",
					expect_dest_id, packet->dest_id);
				delete packet;
				continue;
			}
			if (packet->len != expect_msg_len)
			{
				PTDbgStr("It is not expect msg len, expect:%d, real:%d!\n",
					expect_msg_len, packet->len);
				delete packet;
				continue;
			}
			if (((uint8_t *)(packet->p_data))[0] != expect_msg_type)
			{
				PTDbgStr("It is not expect msg type, expect:%02x, real:%02x!\n",
					expect_msg_type, ((uint8_t *)(packet->p_data))[0]);
				delete packet;
				continue;
			}
			memcpy(o_pdata, packet->p_data, packet->len);
			delete packet;
			break;
		}
		
		Sleep(1);
	}

	if (is_canceled)
	{
		PTDbgStr("Canceled!\n");
		return RECEIVE_PACKET_CANCELED;
	}
	else if (timeout <= 0)
	{
		PTDbgStr("Timeout!\n");
		return RECEIVE_PACKET_TIMEOUT;
	}
	else
	{
		return NO_PT_ERROR;
	}
}

void StartCanRecv()
{
	StartRecvFromCAN();
	StartCanObj();
}

void StopCanRecv()
{
	StopRecvFromCAN();
	StopCanObj();
}


int PTGetMode(uint8_t src, uint8_t dst, IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_parameter, PT_MODE *mode)
{
	int ret = NO_PT_ERROR;
	uint8_t data_received[41] = {0};

	/* 1. Send the factory mode query */
	uint8_t data[2] = {0};
	data[0] = COMMON_SYSCTRL_PROD_MODE_QUERY;
	data[1] = 0x02;

	ret = SendCanData(MIDDLE, src, dst, data, 2);
	if (ret < 0)
	{
		goto out;
	}

	/* 2. Receive the mode to query */
	memset(data_received, 0, sizeof(data_received));
	ret = ReceiveCanData(dst, src, 3,
		COMMON_SYSCTRL_PROD_MODE_QUERY_REP, data_received,
		3000, cancel_callback,
		cancel_parameter);
	if (ret < 0)
	{
		goto out;
	}

	if (mode)
		*mode = (PT_MODE) data_received[2];
out:
	return ret;
}

int PTSwitchMode(uint8_t src, uint8_t dst, IS_CANCEL_CALLBACK cancel_callback,
	void *cancel_parameter)
{
	int ret = NO_PT_ERROR;
	uint8_t data_received[41] = {0};

	/* 1. Send the swtiching mode */
	uint8_t data[2] = {0};
	data[0] = COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD;
	data[1] = 0x02;

	ret = SendCanData(MIDDLE, src, dst, data, 2);
	if (ret < 0)
	{
		goto out;
	}

	/* 2. Receive the response of switching mode */
	memset(data_received, 0, sizeof(data_received));
	ret = ReceiveCanData(dst, src, 3,
		COMMON_SYSCTRL_SWITCH_PROD_MODE_CMD_REP, data_received,
		3000, cancel_callback,
		cancel_parameter);
	if (ret < 0)
	{
		goto out;
	}

	if (data_received[2] != 0)
	{
		ret = SWITCH_MODE_FAILED;
	}

out:
	return ret;
}

DLL_API int PTSelectCanChannel(enum CAN_CHANNEL can_channel)
{
	if (can_channel != CHANNEL_ONE && CHANNEL_ONE != CHANNEL_TWO)
	{
		return INVALID_PARAMETER;
	}
	canind = can_channel;

	return NO_PT_ERROR;
}