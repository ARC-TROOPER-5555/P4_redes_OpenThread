/*
 * Copyright 2025 NXP
 * 
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <openthread/coap.h>
#include <openthread/cli.h>
#include "LED.h"
#include "Temp_sensor.h"
#include <stdio.h>
#include <stdlib.h>

#include "Coap_Server.h"
#include <ctype.h>
#include <string.h>

otInstance *instance_g;

#define NAME_MAX_LEN 20
static char nombre_resource[NAME_MAX_LEN] = "Sin Nombre";


void handle_led_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    char payload[10];
    int length = otMessageRead(aMessage, otMessageGetOffset(aMessage), payload, sizeof(payload) - 1);
    payload[length] = '\0';

    if (payload[0] == '1')
    {
        // Turn LED on
        otCliOutputFormat("Payload Recived: %s\r\n", payload);
        otCliOutputFormat("LED On \r\n");
        LED_ON();

    }
    else if (payload[0] == '0')
    {
        // Turn LED off
        otCliOutputFormat("Payload Recived: %s\r\n", payload);
        otCliOutputFormat("LED Off \r\n");
        LED_OFF();
    }

    //Send response
    otMessage *response = otCoapNewMessage(instance_g, NULL);
    otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
    otCoapSendResponse(instance_g, response, aMessageInfo);
}

void handle_sensor_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    static double temp_value = 0;
    temp_value = Get_Temperature();

    otMessage *response;

    if (otCoapMessageGetCode(aMessage) == OT_COAP_CODE_GET)  
    {
        response = otCoapNewMessage(instance_g, NULL);
        otCliOutputFormat("GET\r\n");

        if (response != NULL)
        {
            otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CONTENT);
            
            otCoapMessageSetPayloadMarker(response);
           
            char sensorData[50] = {"0"};
            
            snprintf(sensorData, sizeof(sensorData), "%d", (int)temp_value);
            otCliOutputFormat("payload: %s\r\n", sensorData);

            otMessageAppend(response, sensorData, strlen(sensorData));

            otCoapSendResponse(instance_g, response, aMessageInfo);
        }
    }
}
//Nuevo recurso
void handle_nombre_request(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    otMessage *response = NULL;
    otCoapCode code = otCoapMessageGetCode(aMessage);

    switch (code)
    {
    case OT_COAP_CODE_GET:
        otCliOutputFormat("GET /nombre\r\n");
        otCliOutputFormat("Payload actual: %s\r\n", nombre_resource);
        response = otCoapNewMessage(instance_g, NULL);
        if (response != NULL)
        {
            otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CONTENT);
            otCoapMessageSetPayloadMarker(response);
            otMessageAppend(response, nombre_resource, strlen(nombre_resource));
            otCoapSendResponse(instance_g, response, aMessageInfo);
        }
        break;

    case OT_COAP_CODE_PUT:
    {
        char new_name[NAME_MAX_LEN];
        int length = otMessageRead(aMessage, otMessageGetOffset(aMessage), new_name, sizeof(new_name) - 1);
        new_name[length] = '\0';

        otCliOutputFormat("PUT /nombre\r\n");
        otCliOutputFormat("Payload recibido: %s\r\n", new_name);

        bool valid = true;
        for (int i = 0; i < length; i++)
        {
            if (!isalpha(new_name[i]) && !isspace(new_name[i]))
            {
                valid = false;
                break;
            }
        }

        response = otCoapNewMessage(instance_g, NULL);
        if (response != NULL)
        {
            if (valid)
            {
                strncpy(nombre_resource, new_name, NAME_MAX_LEN - 1);
                nombre_resource[NAME_MAX_LEN - 1] = '\0';
                otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
            }
            else
            {
                otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_BAD_REQUEST);
            }
            otCoapSendResponse(instance_g, response, aMessageInfo);
        }
        break;
    }

    case OT_COAP_CODE_DELETE:
        strncpy(nombre_resource, "Sin Nombre", NAME_MAX_LEN);
        response = otCoapNewMessage(instance_g, NULL);
        if (response != NULL)
        {
            otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
            otCoapSendResponse(instance_g, response, aMessageInfo);
        }
        break;

    default:
        response = otCoapNewMessage(instance_g, NULL);
        if (response != NULL)
        {
            otCoapMessageInitResponse(response, aMessage, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_METHOD_NOT_ALLOWED);
            otCoapSendResponse(instance_g, response, aMessageInfo);
        }
        break;
    }
}


void init_coap_server(otInstance *aInstance)
{
    I2C2_InitPins();
    LED_INIT();
    Temp_Sensor_start();

    instance_g = aInstance;
    
    static otCoapResource coapResource_led;
    static otCoapResource coapResource_sensor;
    static otCoapResource coapResource_nombre;

    
    coapResource_led.mUriPath = "led";
    coapResource_led.mHandler = handle_led_request;
    coapResource_led.mContext = NULL;
    coapResource_led.mNext = NULL;

    otCoapAddResource(aInstance, &coapResource_led);

    coapResource_sensor.mUriPath = "sensor";
    coapResource_sensor.mHandler = handle_sensor_request;
    coapResource_sensor.mContext = NULL;
    coapResource_sensor.mNext = NULL;

    otCoapAddResource(aInstance, &coapResource_sensor);

    //Nuevo recurso
    coapResource_nombre.mUriPath = "nombre";
    coapResource_nombre.mHandler = handle_nombre_request;
    coapResource_nombre.mContext = NULL;
    coapResource_nombre.mNext = NULL;

    otCoapAddResource(aInstance, &coapResource_nombre);
}