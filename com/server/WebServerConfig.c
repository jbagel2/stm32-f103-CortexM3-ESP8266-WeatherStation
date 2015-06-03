
#include "WebServerConfig.h"



char webResponse[RESPONSE_BUFFER_SIZE];

const char *ValidQueryStrings[4] = {"Dim","Reset","Settings","Status"}; // Query strings that are valid

void Update_State_Variables(KeyValuePair_String_Uint16_t newStates)
{

	if(strstr(newStates.key, "pressure"))
	{
		pressure_Current = newStates.value;
	}
	else if(strstr(newStates.key, "altitude"))
	{
		altitude_Current = newStates.value;
	}
	else if(strstr(newStates.key, "altTemp"))
		{
		altTemp_Current = newStates.value;
		}
}
