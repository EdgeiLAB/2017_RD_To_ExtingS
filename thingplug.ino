bool connectingThingplug() {
  if(!mqttConnect(&mqttClient, addr, id, pw, deviceId)) {
    printf("1. mqtt connect failed\n");
    return false;
  }

  if(!mqttCreateNode(&mqttClient, devicePw)) {
    printf("2. mqtt create node failed\n");
    return false;    
  }

  if(!mqttCreateRemoteCSE(&mqttClient)) {
    printf("3. mqtt create remote cse failed\n");
    return false;    
  }  
  
  if(!mqttCreateContainer(&mqttClient, containerGeolocation_latitude)) {
    printf("4. mqtt create container failed\n");
    return false;
  }

  if(!mqttCreateContainer(&mqttClient, containerGeolocation_longitude)) {
    printf("4. mqtt create container failed\n");
    return false;
  }
/*
  if(!mqttSubscribe(&mqttClient, targetDeviceId, containerSmoke, callbackFunctionSmoke)) {
    printf("6. mqtt subscribe notification failed\n");
    return false;
  }
*/
//  if(!mqttGetLatest(&mqttClient, targetDeviceId, containerSmoke, callbackFunctionSmoke))

 
  return true;
}

void mqttPublish_Geolocation() {
  Serial.println("Publish Geolocation");
  char strLatitude[BUF_SIZE_SMALL];
  char strLongitude[BUF_SIZE_SMALL];
  sprintf(strLatitude, "%f", latitude);
  sprintf(strLongitude, "%f", longitude);

  if(isConnectingWifi()) {
    mqttCreateContentInstance(&mqttClient, containerGeolocation_latitude, strLatitude);
    mqttCreateContentInstance(&mqttClient, containerGeolocation_longitude, strLongitude);    
  }
  else {
    connectingWifi();
    connectingThingplug();
    mqttCreateContentInstance(&mqttClient, containerGeolocation_latitude, strLatitude);
    mqttCreateContentInstance(&mqttClient, containerGeolocation_longitude, strLongitude);        
  }
  memset(strLatitude, 0, BUF_SIZE_SMALL);
  memset(strLongitude, 0, BUF_SIZE_SMALL);  
}

void callbackFunctionSmoke(char * data)
{
  String tmp = String(data);
  printf("callbackFunction: tmp=%s\n", data);  
  
  if(tmp.equals("FIRE_START") || tmp.equals("TEST_START")) {
    flagWarning = true;
    flagSwitch = true;
  }
  else if(tmp.equals("STOP")) {
    flagWarning = false;
    flagSwitch = false;
  }
} 
