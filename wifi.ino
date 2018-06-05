void connectingWifi() {
  Serial.println();
  Serial.println("------ WiFi Connecting ------");
  
  // Connect to WPA/WPA2 network
  WiFi.mode(WIFI_STA);  
  WiFi.begin(ssid, passwd);  
  
  // 연결 완료 까지 10초 대기 
  for(int i=0; i<10; i++) {
    if(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    else  break;
  }
}

bool isConnectingWifi() {
  if(WiFi.status() != WL_CONNECTED) {
    return false;
  }
  return true;
}

