
//单片机头文件
#include "stm32f10x.h"

//网络设备
#include "esp8266.h"

//协议文件
#include "onenet.h"
#include "mqttkit.h"

//硬件驱动
#include "usart.h"
#include "delay.h"

//C库
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "cJSON.h"

/*产品ID*/
#define PROID		"7JRHZ9cq38"

//鉴权Token
#define TOKEN	"version=2018-10-31&res=products%2F7JRHZ9cq38%2Fdevices%2Ftest&et=1803273757&method=md5&sign=Vjklk5bZHFal%2FJWc03vcHw%3D%3D"

//设备名称
#define DEVID		"test"

extern unsigned char esp8266_buf[512];
extern int Ph_L,Ph_H,Tem_L,Tem_H,Lig_L,Lig_H;
extern bool Warning;
//==========================================================
//	函数名称：	OneNet_DevLink
//
//	函数功能：	与onenet创建连接
//
//	入口参数：	无
//
//	返回参数：	1-成功	0-失败
//
//	说明：		与onenet平台建立连接
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//协议包

	unsigned char *dataPtr;
	int i = 0;
	_Bool status = 1;
	
	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
							"PROID: %s,	TOKEN: %s, DEVID:%s\r\n"
                        , PROID, TOKEN, DEVID);
	
	if(MQTT_PacketConnect(PROID, TOKEN, DEVID, 256, 1, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//上传平台
		dataPtr = ESP8266_GetIPD(250);									//等待平台响应

		if(dataPtr != NULL)
		{	
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:UsartPrintf(USART_DEBUG, "Tips:	连接成功\r\n");status = 0;break;
					
					case 1:UsartPrintf(USART_DEBUG, "WARN:	连接失败：协议错误\r\n");break;
					case 2:UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法的clientid\r\n");break;
					case 3:UsartPrintf(USART_DEBUG, "WARN:	连接失败：服务器失败\r\n");break;
					case 4:UsartPrintf(USART_DEBUG, "WARN:	连接失败：用户名或密码错误\r\n");break;
					case 5:UsartPrintf(USART_DEBUG, "WARN:	连接失败：非法链接(比如token非法)\r\n");break;
					
					default:UsartPrintf(USART_DEBUG, "ERR:	连接失败：未知错误\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//删包
	}
	else
		UsartPrintf(USART_DEBUG, "WARN:	MQTT_PacketConnect Failed\r\n");
	
	return status;
	
}

//==========================================================
//	函数名称：	OneNet_Subscribe
//
//	函数功能：	订阅
//
//	入口参数：	topics：订阅的topic
//				topic_cnt：topic个数
//
//	返回参数：	SEND_TYPE_OK-成功	SEND_TYPE_SUBSCRIBE-需要重发
//
//	说明：		
//==========================================================
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	
	unsigned char i = 0;
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包
	
	for(; i < topic_cnt; i++)
		UsartPrintf(USART_DEBUG, "Subscribe Topic: %s\r\n", topics[i]);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, topics, topic_cnt, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);					//向平台发送订阅请求
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}

}

//==========================================================
//	函数名称：	OneNet_Publish
//	函数功能：	发布消息并检测连接状态
//	返回参数：	0-成功，1-失败（网络断开或超时）
//==========================================================
unsigned char OneNet_Publish(const char *topic, const char *msg)
{
	unsigned char status = 1; // 默认设为 1（发送失败）
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};							//协议包
	
	UsartPrintf(USART_DEBUG, "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg),MQTT_QOS_LEVEL0, 0, 1, &mqttPacket) == 0)
	{
		// 【修复核心】：捕获底层 ESP8266 模块的发送结果！
		// 大多数标准库中，如果模块掉线、TCP断开，SendData 会超时并返回非0错误码。
		if( ESP8266_SendData(mqttPacket._data, mqttPacket._len) == 0 )
		{
			status = 0; // 返回 0 代表数据真正从 WiFi 模块成功发出去了
		}
		else
		{
			status = 1; // 底层返回失败，说明 ESP8266 可能掉电或断网了
		}
		
		MQTT_DeleteBuffer(&mqttPacket);											//删包
	}
	
	return status; // 将发送结果传递给 main.c
}

//==========================================================
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};								//协议包
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char type = 0;
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	
	short result = 0;
	cJSON *json, *params_json, *Ph_L_json, *Ph_H_json, *Tem_L_json, *Tem_H_json, *Lig_L_json, *Lig_H_json,*Warning_json;
	char *dataPtr = NULL;
	char numBuf[10];
	int num = 0;
	
	type = MQTT_UnPacketRecv(cmd);
	switch(type)
	{
		case MQTT_PKT_CMD:															//命令下发
			
			result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);	//解出topic和消息体
			if(result == 0)
			{
				UsartPrintf(USART_DEBUG, "cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
				
				if(MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)	//命令回复组包
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send CmdResp\r\n");
					
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//回复命令
					MQTT_DeleteBuffer(&mqttPacket);									//删包
				}
			}
		
		break;
		case MQTT_PKT_PUBLISH:														//接收的Publish消息
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				UsartPrintf(USART_DEBUG, "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
																	cmdid_topic, topic_len, req_payload, req_len);
				
				// 1. 开始 JSON 解析，并进行内存分配失败保护
				json = cJSON_Parse(req_payload);
				if(json == NULL)
				{
					UsartPrintf(USART_DEBUG, "Error: cJSON Parse Failed! Check Heap_Size!\r\n");
				}
				else
				{
					// 2. 安全获取 params 根节点
					params_json = cJSON_GetObjectItem(json, "params");
					if(params_json != NULL)
					{
						// 3. 安全获取各个子节点指针
						Ph_L_json  = cJSON_GetObjectItem(params_json, "Ph_L");
						Ph_H_json  = cJSON_GetObjectItem(params_json, "Ph_H");
						Tem_L_json = cJSON_GetObjectItem(params_json, "Tem_L");
						Tem_H_json = cJSON_GetObjectItem(params_json, "Tem_H");
						Lig_L_json = cJSON_GetObjectItem(params_json, "Lig_L");
						Lig_H_json = cJSON_GetObjectItem(params_json, "Lig_H");
						Warning_json = cJSON_GetObjectItem(params_json, "Warning");
						
						// 4. 提取真实的数值 (三目运算符防崩溃：如果云端没发这个字段，就给个默认值)
						int ph_l_val  = (Ph_L_json != NULL)  ? Ph_L_json->valueint  : Ph_L;
						int ph_h_val  = (Ph_H_json != NULL)  ? Ph_H_json->valueint  : Ph_H;
						int tem_l_val = (Tem_L_json != NULL) ? Tem_L_json->valueint : Tem_L;
						int tem_h_val = (Tem_H_json != NULL) ? Tem_H_json->valueint : Tem_H;
						int lig_l_val = (Lig_L_json != NULL) ? Lig_L_json->valueint : Lig_L;
						int lig_h_val = (Lig_H_json != NULL) ? Lig_H_json->valueint : Lig_H;
						
						// 5. 特殊处理布尔值 (Warning: false/true)
						int warning_val = 0;
						if(Warning_json != NULL)
						{
							// cJSON 内部把 true 宏定义为 cJSON_True，把 false 定义为 cJSON_False
							if(Warning_json->type == cJSON_True) warning_val = 1;
							else warning_val = 0;
						}

						// 6. 完美打印出真实的数值
						UsartPrintf(USART_DEBUG, "Parse OK! Ph_L:%d, Ph_H:%d, Tem_L:%d, Tem_H:%d", 
												 ph_l_val, ph_h_val, tem_l_val, tem_h_val);
						UsartPrintf(USART_DEBUG, "Lig_L:%d, Lig_H:%d, Warning:%d\r\n", 
												 lig_l_val, lig_h_val, warning_val);
						
						// =========================================================
						// 在这里把解析出来的值，赋给你的全局变量
						Ph_L=ph_l_val; Ph_H=ph_h_val; Tem_L=tem_l_val; Tem_H=tem_h_val; Lig_L=lig_l_val; Lig_H=lig_h_val; Warning=warning_val;
						// =========================================================
					}
					
					// =========================================================
					// 新增核心逻辑：提取消息 ID 并向 OneNET 回复确认包 (ACK)
					// =========================================================
					cJSON *id_json = cJSON_GetObjectItem(json, "id");
					if(id_json != NULL && id_json->type == cJSON_String)
					{
						char reply_topic[100];
						char reply_payload[100];
						
						// 组装应答 Topic (必须是 set_reply 主题)
						sprintf(reply_topic, "$sys/7JRHZ9cq38/test/thing/property/set_reply");
						
						// 组装应答 Payload，格式必须为 {"id":"云端发来的流水号","code":200,"msg":"success"}
						sprintf(reply_payload, "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}", id_json->valuestring);
						
						// 发送确认包给云端
						OneNet_Publish(reply_topic, reply_payload);
						
						// 串口打印反馈，方便你在电脑上观察是否发送成功
						UsartPrintf(USART_DEBUG, "Tips: Send ACK OK! payload: %s\r\n", reply_payload);
					}

					cJSON_Delete(json);
				}
			}
		break;
			
		case MQTT_PKT_PUBACK:														//发送Publish消息，平台回复的Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Publish Send OK\r\n");
			
		break;
			
		case MQTT_PKT_PUBREC:														//发送Publish消息，平台回复的Rec，设备需回复Rel消息
		
			if(MQTT_UnPacketPublishRec(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRec\r\n");
				if(MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send PublishRel\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
			
		case MQTT_PKT_PUBREL:														//收到Publish消息，设备回复Rec后，平台回复的Rel，设备需再回复Comp
			
			if(MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishRel\r\n");
				if(MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
				{
					UsartPrintf(USART_DEBUG, "Tips:	Send PublishComp\r\n");
					ESP8266_SendData(mqttPacket._data, mqttPacket._len);
					MQTT_DeleteBuffer(&mqttPacket);
				}
			}
		
		break;
		
		case MQTT_PKT_PUBCOMP:													//发送Publish消息，平台返回Rec，设备回复Rel，平台再返回的Comp
		
			if(MQTT_UnPacketPublishComp(cmd) == 0)
			{
				UsartPrintf(USART_DEBUG, "Tips:	Rev PublishComp\r\n");
			}
		
		break;
			
		case MQTT_PKT_SUBACK:														//发送Subscribe消息的Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe OK\r\n");
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT Subscribe Err\r\n");
		
		break;
			
		case MQTT_PKT_UNSUBACK:													//发送UnSubscribe消息的Ack
		
			if(MQTT_UnPacketUnSubscribe(cmd) == 0)
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe OK\r\n");
			else
				UsartPrintf(USART_DEBUG, "Tips:	MQTT UnSubscribe Err\r\n");
		
		break;
		
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();									//清空缓存
	
	if(result == -1)
		return;
	
	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}

}
