

#ifndef _WEBSERVERCONFIG_H_
#define _WEBSERVERCONFIG_H_

#include "globalDefines.h"
#include "KeyValuePair.h"
#include <string.h>

//#define RESPONSE_BUFFER_SIZE 1000

//Modify this section to reflect your valid rest query strings

typedef enum
{
	OFF,
	ON
}On_Off;


uint16_t pressure_Current;
uint16_t altitude_Current;
uint16_t altTemp_Current;

const typedef enum {Dim,Reset,Settings,Status}REST_ValidQueryStrings; // The query strings that are accepted by the rest call
const char *ValidQueryStrings[4]; // Query strings that are valid
void Update_State_Variables(KeyValuePair_String_Uint16_t newStates);



#endif // _WEBSERVERCONFIG_H_
