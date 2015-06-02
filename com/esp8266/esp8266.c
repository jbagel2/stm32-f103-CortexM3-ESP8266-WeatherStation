
/********************************************************
 * Author: Jacob Pagel
 * Date: 03/05/2015
 * esp8266.c (esp8266 wifi library source file)
 ********************************************************/

#include "com/esp8266/esp8266.h"

//#include "swamp_controls/swamp_functions.h"
//#include "dht22.h"


char *ESP_IPD_Data_Buffer_Pntr;
char ESP_IPD_DataBuffer[RxBuffSize];

char *ESP_Ready_Buffer_Pntr;
char ESP_Ready_Buffer[RxBuffSize];


char commandToSend[70];
volatile uint8_t waitingForReponse = 0;
volatile uint8_t OKFound = 0;
volatile uint8_t ERRORFound = 0;
volatile uint32_t TxWaitForResponse_TimeStmp = 0;
//extern volatile char USART3_RxBuffer_Buffer[RxBuffSize];
extern volatile char USART3_RxBuffer[RxBuffSize];
//extern DHT22_Data Current_DHT22_Reading;

uint8_t pumpModeToValidate = -1; //Just a starting value that is outside the allowed
uint8_t fanModeToValidate = -1;

//extern On_Off pumpMode_Current;
//extern Fan_Mode fanMode_Current;
//extern On_Off fanLow_Current;
//extern On_Off fanHigh_Current;
//extern uint8_t temp_Current;
//extern uint8_t humid_Current;

extern uint32_t lastDMABuffPoll;


uint16_t incommingDimmingValue = 0;
char *dimmingString;
char *URI;
char *queryString1;
char *queryValue1;
char *queryString2;
char *queryValue2;

IPD_Data currentIPD;

//METHOD DECLARATIONS
IPD_Data ProcessIPD_Data(char *IPD_Buffer);



const char *ATCommandsArray[19] = {
	"AT",
	"AT+CIPSTATUS",
	"AT+CWLAP",
	"AT+GMR",
	"AT+CWMODE?",
	"AT+CWMODE=3",
	"AT+CWJAP=\"Nonya\",\"porsche911\"",
	"AT+CWJAP?",
	"AT+RST",
	"AT+CIPMUX=1",
	"AT+CIOBAUD=115200",
	"AT+CIPSERVER=1,80",
	"AT+CIFSR",
	"AT+CIPSTART=?",
	"AT+CWLIF",
	"AT+CWQAP",
	"AT+CWSAP=",
	"ATE0",
	"AT+CIPCLOSE="};


const char *ESP_Responses[10] =
{
		"ready",
		"Link",
		"Unlink",
		"OK",
		"SEND OK",
		"+IPD",
		"ERROR",
		"wrong syntax",
		"busy p...",
		"busy inet..."
};



void Wifi_Init()
{
	//ESP Control pin (CH_PD) config
	GPIO_InitTypeDef ESP8266_Control_Config; //Does not include USART thats taken care of with USART
	ESP8266_Control_Config.GPIO_Speed = GPIO_Speed_50MHz;
	ESP8266_Control_Config.GPIO_Mode = GPIO_Mode_Out_PP;
	ESP8266_Control_Config.GPIO_Pin = ESP_CHPD_Pin;
	GPIO_Init(ESP_CHPD_Control_GPIO, &ESP8266_Control_Config);
#define WIFI_INITIALIZED
}

void Wifi_OFF()
{
#ifndef WIFI_INITIALIZED
#error You must initialize Wifi first with Wifi_Init()
#endif
	ESP_CHPD_Control_GPIO->BRR = ESP_CHPD_Pin;
}

void Wifi_ON()
{
#ifndef WIFI_INITIALIZED
#error You must initialize Wifi first with Wifi_Init()
#endif
	ESP_CHPD_Control_GPIO->BSRR = ESP_CHPD_Pin;
}


void ClearArray_Size(char buffer[], uint16_t size)
{
	memset(buffer, '\0', size);
}

void SetArray_Size(char buffer[], uint16_t size)
{
	memset(buffer, '1', size);
}

void Wifi_ReadyWaitForAnswer()
{
	TxWaitForResponse_TimeStmp = Millis();
	waitingForReponse = 1;

}

void Wifi_WaitForAnswer()
{
	while(waitingForReponse == 1 && (Millis() - TxWaitForResponse_TimeStmp) < ESP_ResponseTimeout_ms);
	OKFound=0;
	ERRORFound=0;
}


char *WaitForAnswer_cmd_Buffer;
char *WaitForAnswer_ans_Buffer;
///Will parse the USART buffer periodically (based on #defined poll interval) for the echo of cmdToWaitFor
///in the response from the ESP8266 module.
//NEED TO PULL OUT CIFSR STUFF so it doesnt just get deleted
void Wifi_WaitForAnswerCMD(char *cmdToWaitFor, uint16_t cmdSize)
{

	while(waitingForReponse == 1 && (Millis() - TxWaitForResponse_TimeStmp) < ESP_ResponseTimeout_ms)
		{
		WaitForAnswer_cmd_Buffer = memmem(USART3_RxBuffer,RxBuffSize,cmdToWaitFor,cmdSize);
		if(strlen(WaitForAnswer_cmd_Buffer)>0)
		{
			if((WaitForAnswer_ans_Buffer = memmem(WaitForAnswer_cmd_Buffer,strlen(WaitForAnswer_cmd_Buffer),"OK\r\n",4)))
			{
				ClearArray_Size(WaitForAnswer_cmd_Buffer, strlen(WaitForAnswer_cmd_Buffer));
				OKFound=1;
				waitingForReponse = 0;
			}
			//Check for OK or Error Message

		}

		};
	//OKFound=0;
	//ERRORFound=0;
}

uint32_t pointerRange = 0;
void Wifi_WaitForAnswer_SEND_OK(uint16_t cmdSize)
{

	while(waitingForReponse == 1 && (Millis() - TxWaitForResponse_TimeStmp) < ESP_ResponseTimeout_ms)
	{
		WaitForAnswer_cmd_Buffer = memmem(USART3_RxBuffer,RxBuffSize,"AT+CIPSEND",10);
		if(strlen(WaitForAnswer_cmd_Buffer)>0)
		{
			while(waitingForReponse == 1 && (Millis() - TxWaitForResponse_TimeStmp) < ESP_ResponseTimeout_ms)
				{
				if((WaitForAnswer_ans_Buffer = memmem(USART3_RxBuffer,strlen(USART3_RxBuffer),"SEND OK\r\n",9)))
				{
					pointerRange = WaitForAnswer_cmd_Buffer - WaitForAnswer_ans_Buffer;
					ClearArray_Size(WaitForAnswer_cmd_Buffer, cmdSize + 9);
					OKFound=1;
					waitingForReponse = 0;
				}
				}
			//Check for OK or Error Message
		}
	}

}


char closeConnectionBuffer[15];
void Wifi_CloseConnection(uint8_t connectionNum)
{
	sprintf(closeConnectionBuffer, "AT+CIPCLOSE=%d",connectionNum);
	Wifi_SendCustomCommand(closeConnectionBuffer);
}


void Wifi_SendCustomCommand(char *customMessage)
{
			Wifi_SendCustomCommand_External_Wait(customMessage);

			Wifi_WaitForAnswer();
			//for (wi=0;wi<735000;wi++);
}

void Wifi_SendCustomCommand_External_Wait(char *customMessage)
{
		while(*customMessage)
		{
			while(USART_GetFlagStatus(ESP_USART,USART_FLAG_TXE) == RESET);
			USART_SendData(ESP_USART,*customMessage++);
		}

		while(USART_GetFlagStatus(ESP_USART, USART_FLAG_TXE) == RESET);
			USART_SendData(ESP_USART,'\r');

		while(USART_GetFlagStatus(ESP_USART, USART_FLAG_TXE) == RESET);
			Wifi_ReadyWaitForAnswer();
			USART_SendData(ESP_USART,'\n');

			//Wifi_WaitForAnswer();
			//for (wi=0;wi<735000;wi++);
}

//Waits to return untill wifi responds (OK or ERROR)
void Wifi_SendCommand(Wifi_Commands command )
{
	const char *commandToSend = ATCommandsArray[command];

	while(*commandToSend)
	{
		while(USART_GetFlagStatus(ESP_USART,USART_FLAG_TXE) == RESET);
		USART_SendData(ESP_USART,*commandToSend++);
	}
	Wifi_ReadyWaitForAnswer();

	while(USART_GetFlagStatus(ESP_USART, USART_FLAG_TXE) == RESET);
	USART_SendData(ESP_USART,'\r');

	//Wifi_ReadyWaitForAnswer();
	while(USART_GetFlagStatus(ESP_USART, USART_FLAG_TXE) == RESET);

	USART_SendData(ESP_USART,'\n');

	if(command == WIFI_GET_CURRENT_IP)
	{
		Wifi_WaitForAnswerCMD("OK\r\n", 4);
	}
	else
	{
		Wifi_WaitForAnswerCMD(ATCommandsArray[command], strlen(ATCommandsArray[command]));
	}

	//for (wi=0;wi<735000;wi++);

}

char *IPD_Processing_buf;
char *ConnectNum;
//Breaks the IPD message into a proper request object
IPD_Data ProcessIPD_Data(char *IPD_Buffer)
{
	//IPD_Processing_buf = strdupa(IPD_Buffer);
	//IPD_Processing_buf = &IPD_Buffer + 5;
	IPD_Data thisIPDMessage;

	strtok(IPD_Buffer,",");

	ConnectNum = strtok(NULL,",");
	thisIPDMessage.ConnectionNum = atoi(ConnectNum);

	thisIPDMessage.DataSize = strtok(NULL,":");
	//TODO: Probably need to add a check to make sure actual datasize matches expected..

	thisIPDMessage.RequestType = strtok(NULL," ");

	thisIPDMessage.URI = strtok(NULL," ");

	strtok(NULL,"\r\n");

	thisIPDMessage.Headers = strtok(NULL,"{");

	thisIPDMessage.Body = strtok(NULL,"}");
	return thisIPDMessage;

}

//Find first array index that contains the stringToFind
uint16_t IndexOf(char *arrayToSearch[], uint16_t arraySize,char *stringToFind)
{
	uint16_t correctedSize = arraySize / sizeof(int); //NEED TO EVALUATE IF sizeof(int) evaluates as expected
	uint16_t i = 0;
	for(i; i < correctedSize ;i++)
	{
		if(strstr(arrayToSearch[i],stringToFind))
		{
			return i;
		}
	}
	return NULL;
}

//Extracts the enum for the HTTPRequest type of the request
Http_Method_Enum IsRequestType(IPD_Data request)
{
	uint16_t enumValue = IndexOf(HTTP_Method, sizeof(HTTP_Method),request.RequestType);
	if(enumValue != NULL)
	{
		Http_Method_Enum method = enumValue;
		return method;
	}
	return REQUEST_TYPE_ERROR;
}

uint8_t Wifi_CheckDMABuff_ForReady()
{
	uint8_t foundReady = 0;
	ESP_Ready_Buffer_Pntr = memmem(USART3_RxBuffer,RxBuffSize,"ready",5);
	if(ESP_Ready_Buffer_Pntr){
		strcpy(ESP_Ready_Buffer,ESP_Ready_Buffer_Pntr);
		DMA_Cmd(DMA1_Channel3,DISABLE);

		//Wipes the received message from the DMA buffer (using the pointer to the data)
		//This makes sure the data doesn't get mistaken for a new request, on the next buffer polling.
		ClearArray_Size(ESP_Ready_Buffer_Pntr,strlen(ESP_Ready_Buffer_Pntr));
		DMA_Initialize(USART3_RxBuffer, RxBuffSize);
		foundReady = 1;
	}
	return foundReady;
}

char *APIP;
char *APMAC;
char *STAIP;
char *STAMAC;
extern ESP_Status currentESPStatus;

void Wifi_CheckDMABuff_ForCIFSRData()
{

	ESP_Ready_Buffer_Pntr = memmem(USART3_RxBuffer,RxBuffSize,"+CIFSR:APIP",11);
	if(ESP_Ready_Buffer_Pntr){
		strcpy(ESP_Ready_Buffer,ESP_Ready_Buffer_Pntr);
		DMA_Cmd(DMA1_Channel3,DISABLE);

		//Wipes the received message from the DMA buffer (using the pointer to the data)
		//This makes sure the data doesn't get mistaken for a new request, on the next buffer polling.
		ClearArray_Size(ESP_Ready_Buffer_Pntr,strlen(ESP_Ready_Buffer_Pntr));
		DMA_Initialize(USART3_RxBuffer, RxBuffSize);

		strtok(ESP_Ready_Buffer, ","); //Discard the '+CIFSR:APIP'
		currentESPStatus.AccessPoint_IP = strtok(NULL, "\r");
		strtok(NULL, ","); //Discard the '+CIFSR:APMAC'
		currentESPStatus.AccessPoint_MAC = strtok(NULL, "\r");
		strtok(NULL, ","); //Discard the '+CIFSR:STAIP'
		currentESPStatus.Station_IP = strtok(NULL, "\r");
		strtok(NULL, ","); //Discard the '+CIFSR:STAMAC'
		currentESPStatus.Station_MAC = strtok(NULL, "\r");
	}

}


IPD_Data Wifi_CheckDMABuff_ForIPDData(DHT22_Data *Current_DHT22_Reading)
{
	currentIPD.Valid = 0;

	//Probably need to check for new client ({clientNum},CONNECT)
	lastDMABuffPoll = Millis();
	ESP_IPD_Data_Buffer_Pntr = memmem(USART3_RxBuffer,RxBuffSize,"+IPD",4);
	if(ESP_IPD_Data_Buffer_Pntr)
	{
		//position = DMA_GetCurrDataCounter(DMA1_Channel3);
		//position = strlen(USART3_RxBuffer);
		//Copy IPD message and data to its own buffer so DMA can go about its business
		strcpy(ESP_IPD_DataBuffer,ESP_IPD_Data_Buffer_Pntr);
		DMA_Cmd(DMA1_Channel3,DISABLE);

		//Wipes the received message from the DMA buffer (using the pointer to the data)
		//This makes sure the data doesn't get mistaken for a new request, on the next buffer polling.
		ClearArray_Size(ESP_IPD_Data_Buffer_Pntr,strlen(ESP_IPD_Data_Buffer_Pntr));
		DMA_Initialize(USART3_RxBuffer, RxBuffSize);


		//now we process since DMA isn't going to stomp on us.
		currentIPD = ProcessIPD_Data(ESP_IPD_DataBuffer);

		Http_Method_Enum requestType = IsRequestType(currentIPD);

		if(requestType == POST)
		{
			char *newStart = strchr(currentIPD.URI,'/');
			if(newStart)
			{
				//If the URI starts with a '/' we get rid of it by moving the pointer up by one mem address
				currentIPD.URI = newStart + 1;
			}
			//Make sure its actually a query
			if(strstr(currentIPD.URI, "?"))
			{
			//Check for multiple query strings
			uint8_t multiQuery = strstr(currentIPD.URI, "&");
			if(multiQuery)
			{
				//TODO: Replace temp queryStringArray size of 4
				KeyValuePair_String_Uint16_t queryStrings[4];
				uint8_t qsc = 0;

				//Get rid of the start '?' char
				URI = strtok(currentIPD.URI,"?");
				for(qsc; qsc < 4; qsc++)
				{
					KeyValuePair_String_Uint16_t thisQuery;
					thisQuery.key = "\0";
					thisQuery.value = -1;
					if(qsc == 0)
					{thisQuery.key = strtok(URI,"=");} else{ thisQuery.key = strtok(NULL,"=");}
					if(thisQuery.key=="\0" || thisQuery.key == NULL)
					{
						if(qsc > 0)
						{
							currentIPD.Valid = 1;
							RefreshCustomRESTResponseSwamp(currentESPStatus.Station_IP, currentESPStatus.AccessPoint_IP, 0, 0,Current_DHT22_Reading);
						}
						return currentIPD;
					}
					else {
						queryValue1 = strtok(NULL, "&");
						thisQuery.value = atoi(queryValue1);
						//TODO: Need to evaluate what are valid values.
						if(thisQuery.value < 3)
						{
							Update_State_Variables(thisQuery);
							currentIPD.Valid = 1;
							queryStrings[qsc] = thisQuery;
						}
					}
					RefreshCustomRESTResponseSwamp(currentESPStatus.Station_IP, currentESPStatus.AccessPoint_IP, 0, 0,Current_DHT22_Reading);
				}
			}
			else {
				KeyValuePair_String_Uint16_t thisQuery;
				URI = strtok(currentIPD.URI,"?");
				thisQuery.key = "\0";
				thisQuery.value = -1;

				thisQuery.key = strtok(URI,"=");
				queryValue1 = strtok(NULL, "&");
				thisQuery.value = atoi(queryValue1);
				//TODO: Need to evaluate what are valid values.
				if(thisQuery.value < 3)
				{
					Update_State_Variables(thisQuery);

					currentIPD.Valid = 1;
				}
			}
			RefreshCustomRESTResponseSwamp(currentESPStatus.Station_IP, currentESPStatus.AccessPoint_IP, 0, 0,Current_DHT22_Reading);

			}
		}
		else if (requestType == GET)
		{
			//TODO: Still need to add parsing of start up ESP data (ip's, MAC, and ready flag )
			RefreshCustomRESTResponseSwamp(currentESPStatus.Station_IP, currentESPStatus.AccessPoint_IP, 0, 0,Current_DHT22_Reading);
			currentIPD.Valid = 1;
			return currentIPD;
		}
	}
				return currentIPD;
}


///Pretty self explanatory
void ConnectToAP(char *apName, char *password) //Will utilize the arguments later, for now static to Nonya
{
	//TODO: Need to add check that ESP is in a compatible client mode
	sprintf(commandToSend,"AT+CWJAP=\"%s\",\"%s\"",apName,password);
	Wifi_SendCustomCommand(commandToSend);
}

//Configures ESP82667 Access Point with given parameters.
void StartLocalAP(char *SSID, char *password, uint8_t channel, Available_Encyption encypt)
{
	sprintf(commandToSend, "AT+CWSAP=\"\",\"\",\"\",\"\"");
}
