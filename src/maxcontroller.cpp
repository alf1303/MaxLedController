#include "helper.h"

Ticker updateSendTicker; //for sending updated status to Application

void update() {//for sending updated status to Application
  formAnswerInfo(PORT_OUT_UPD);
}

//WiFi Settings
IPAddress apAddress(192, 168, 11, 31);
IPAddress apGateway(192, 168, 11, 31);
IPAddress apSubnet(255, 255, 255, 0);

WiFiUDP wifiUDP;

//Creating access point
void connectWiFi_AP() {
  WiFi.enableAP(1);
  WiFi.softAPConfig(apAddress, apGateway, apSubnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(15);
  printf("Creating Access point...\n");
}

//udp server for sending updated status
void startUdpServer() {
  bool res = wifiUDP.begin(PORT_IN);
  if(res == 1) 
    printf("Opened UDP socket on port %d\n", PORT_IN);
  else 
    printf("Error opening UDP socket\n");
  delay(15);
  uni = WiFi.softAPIP()[3]; //id of Esp, same as last byte of its ipaddress
}

//Connecting to Access Point
boolean connectWiFi() {
  boolean state = true;
  int i = 0;
  WiFi.persistent(false);
  WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setOutputPower(17.0); //16.4 20max
  WiFi.enableAP(0);
  if(WiFi.SSID() == settings.network && WiFi.psk() == settings.password) {
    printf("**** Loading WiFi settings from ROM\n");
    WiFi.begin();
  }
  else {
      printf("**** Connecting with new settings\n");
      WiFi.begin(settings.network, settings.password);
  }
  Serial.printf("**** Connecting to WiFi. SSID: %s\n", settings.network);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print("*.");
   if (i > 30){              // Wait 8s for connecting to WiFI
    state = false;
      break;
    }
    i++;
  }
  if(!state) {
    printf("**** Cant connect to WiFi SSID: \n");
    WiFi.disconnect();
  }
  else {
    printf("**** Connected to WiFi! SSID:");
    Serial.print(WiFi.SSID());
    printf(" IP: ");
    Serial.print(WiFi.localIP());
    Serial.println();
  }
  return state;
}

//Reading values, arrived by UDP from Application
void readUDP() {
  if(wifiUDP.parsePacket()) {
    printf("udp in\n");
    wifiUDP.read(hData, 5);
    command = hData[3];
    option = hData[4];
    //printf("ReadUDP header1: 0: %c, 1: %c, 2: %d, 3: %c, 4: %c, 5: %c\n", (char)hData[0], (char)hData[1], hData[2], (char)hData[3], (char)hData[4]);
    if(hData[0] == 'C' && hData[1] == 'P') {
      sourceIP = wifiUDP.remoteIP();
      uni = hData[2];
      if(hData[3] == 'S') {
        //if main settings + pixel settings
        if(hData[4] == 'S') {
          wifiUDP.read(hData1, 13);
          wifiUDP.read(hData2, 17);
          mask = hData1[8];
          request.dimmer = hData1[7];
          request.color = RgbColor(hData1[4], hData1[5], hData1[6]);
          request.fxColor = RgbColor(hData2[7], hData2[8], hData2[9]);
          request.strobe = hData2[10];
          request.fxNumber = hData1[2];
          request.fxSpeed = speedToDouble(hData1[3]);
          request.fxSize = hData2[11];
          request.fxParts = hData2[12];
          if(request.fxParts != settings.fxParts) {
            FX.needRecalculate = true;
          }
          request.fxFade = hData2[13];
          request.fxParams = hData2[14];
          settings.fxParams = request.fxParams;
          request.fxSpread = hData2[15];
          request.fxWidth = hData2[16];
          printf("** udp read, recSize: %d, width: %d\n", request.fxSize, hData2[16]);
          if(request.fxSpread != settings.fxSpread) {
            FX.needRecalculate = true;
          }
          request.pixelCount = hData2[0] + (hData2[4]<<8);
          request.startPixel = hData2[1] + (hData2[5]<<8);
          request.endPixel = hData2[2] + (hData2[6]<<8);
        
        }
        //if name settings
        if(hData[4] == 'N') {
          uint8_t nameSize = wifiUDP.read();
          if(nameSize > 0) {
            settings.name = new char[nameSize+1];
            wifiUDP.read(settings.name, nameSize);
            settings.name[nameSize] = '\0';
            saveNameToFs(false);
          }
        }
        //if network settings for client mode
        if(hData[4] == 'W') {
          settings.netMode = wifiUDP.read();
          uint8_t ssidSize = wifiUDP.read();
          uint8_t passwordSize = wifiUDP.read();
          if(ssidSize > 0 && passwordSize > 0) {
            settings.network = new char[ssidSize+1];
            settings.password = new char[passwordSize+1];
            wifiUDP.read(settings.network, ssidSize);
            wifiUDP.read(settings.password, passwordSize);
            settings.network[ssidSize] = '\0';
            settings.password[passwordSize] = '\0';
            //printf("***received: netMode: %d, ssidSize: %d, passSize: %d\n", settings.netMode, ssidSize, passwordSize);
            saveNetworkDataToFs(false);
            saveSettingsToFs(false);
            //setReset();
          }
        }

        //if pixel settings
        if(hData[4] == 'P') {
          fxTicker.detach();
          uint8_t low = wifiUDP.read();
          uint8_t high = wifiUDP.read();
          //printf("low: %d, high: %d, result: %d\n", low, high, low + (high << 8));
          settings.pixelCount = low + (high << 8);
          settings.endPixel = settings.pixelCount;
          //printf("Before Save: pixelCount: %d, endPixel: %d\n", settings.pixelCount, settings.endPixel);
          saveSettingsToFs(false);
          delete FX.fxData;
          delete FX.fxTemp;
          FX.initFxData();
          //ESP.reset();
          setReset();
        }

        //if playlist settings
        if(hData[4] == 'L') {
          uint8_t plSize = wifiUDP.read();
          playlistPeriod = wifiUDP.read();
          //delete playlist;
          delay(10);
          playlist = new ledsettings_t[plSize];
          playlist_temp = playlist;
          //printf("plSize: %d, plPeriod: %d\n", plSize, playlistPeriod);
          for(int i = 0; i < plSize; i++) {
            ledsettings_t set;
            set.dimmer = wifiUDP.read();
            set.color.R = wifiUDP.read();
            set.color.G = wifiUDP.read();
            set.color.B = wifiUDP.read();
            set.fxColor.R = wifiUDP.read();
            set.fxColor.G = wifiUDP.read();
            set.fxColor.B = wifiUDP.read();
            set.strobe = wifiUDP.read();
            set.fxNumber = wifiUDP.read();
            set.fxSpeed = speedToDouble(wifiUDP.read());
            set.fxSize = wifiUDP.read();
            set.fxParts = wifiUDP.read();
            set.fxFade = wifiUDP.read();
            set.fxParams = wifiUDP.read();
            set.fxSpread = wifiUDP.read();
            set.fxWidth = wifiUDP.read();
            set.fxReverse = (set.fxParams)&1;
            set.fxAttack = (set.fxParams>>1)&1;
            set.fxSymm = (set.fxParams>>2)&1;
            set.fxRnd = (set.fxParams>>3)&1;
            set.fxRndColor = (set.fxParams>>7)&1;
            *playlist = set;
            playlist++;
          }
          playlist = playlist_temp;
          settings.playlistSize = plSize;
          savePlaylist();
        }

      }
      processRequest();
    }
//printf("**udpreceive** name: %s, network: %s, password: %s, count: %d, fxBlue: %d\n", settings.name, settings.network, settings.password, settings.pixelCount, settings.fxColor.B);
//printf("**udpreceive** fxNum: %d, fxSpeed: %f, fxParts: %d, fxR: %d, fxG: %d, fxB: %d\n", settings.fxNumber, settings.fxSpeed, settings.fxParts, settings.fxColor.R, settings.fxColor.G, settings.fxColor.B);
//printf("**udpreceive** fxSpread: %d, fxWidth: %d, fxParams: %d, reqFxParam: %d, fxSize: %d\n", settings.fxSpread, settings.fxWidth, settings.fxParams, request.fxParams, settings.fxSize);
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  LittleFS.begin();
  strip.Begin();
  //LittleFS.format();
  setRandomSsidName();
  initSettings();////
  FX.initFxData();////
  test2();
  if(settings.netMode == 0) {
    connectWiFi_AP();
  }
  else {
    boolean res = connectWiFi();
    if(!res) {
      connectWiFi_AP();
    }
  }
  startUdpServer();
  OTA_Func();
  updateSendTicker.attach(2, update);
  //printf("**setup** name: %s, network: %s, password: %s, count: %d, fxBlue: %d\n", settings.name, settings.network, settings.password, settings.pixelCount, settings.fxColor.B);
  //printf("**setup** pixelCount: %d, startPix: %d, endPix: %d, netMode: %d\n",settings.pixelCount, settings.startPixel, settings.endPixel, settings.netMode);
  //printf("**setup** fxWidth: %d, fxNum: %d, fxSpeed: %f, fxParts: %d, fxSpread: %d, setfxParam: %d\n",settings.fxWidth, settings.fxNumber, settings.fxSpeed, settings.fxParts, settings.fxSpread, settings.fxParams);
}

void loop() {
  readUDP();
  processFx();////
  outToStrip();////
  processPlaylist(); ////
  if(toAnswer) {
    formAnswerInfo(PORT_OUT_UPD);
    toAnswer = false;
  }
  ArduinoOTA.handle();
}

void setRandomSsidName() {
 char mac[64];
 String macaddr = WiFi.macAddress().substring(11);
 strcpy(mac, macaddr.c_str());
 strcat(AP_SSID, mac);
}

void stopFX() {
  if(fxTicker.active()) fxTicker.detach();
        if(fxFadeTicker.active()) fxFadeTicker.detach();
        FX.clearFxData();
        FX.fxRunning = false;
        FX.needRecalculate = true;
        FX.rndShouldGo = -1;
        FX.prevIndex = -1;
        FX.lastPixel = 0;
        delay(40);
}

void processFx() {
  switch(settings.fxNumber) {
    case 0: //Stopped FX
      if(FX.fxRunning) {
        stopFX();
        FX.fxRunning = false;
        FX.needRecalculate = true;
        if(FX.previousFxNum != 0) FX.previousFxNum = 0;
        //printf("Stopped FX...\n");
      }
      break;
    case 1: //Sinus FX
      if(!fxTicker.active()) {
        stopFX();
        fxTicker.attach_ms(1000/FX.fps, sinus);
        FX.fxRunning = true;
        FX.previousFxNum = 1;
      }
      if(fxTicker.active() && FX.previousFxNum != 1) {
        stopFX();
        fxTicker.attach_ms(1000/FX.fps, sinus);
        FX.previousFxNum = 1;
        FX.fxRunning = true;
      }
      break;
    case 2: //Cyclon FX
      if(FX.previousFxNum != 2) {
        stopFX();
        setupAnimations();
        FX.fxRunning = true;
        FX.previousFxNum = 2;
      }
      if(FX.speedChanged) {
        FX.animations.ChangeAnimationDuration(1, (uint16_t)((SPEED_MAX_DOUBLE - settings.fxSpeed)*1000 + 5));
        FX.speedChanged = false;
      }
      FX.animations.UpdateAnimations();
      break;
    case 3: //Fade FX
      if(FX.previousFxNum != 3) {
        //printf("R: %d, G:%d, B:%d, fxSize: %d, fxParts: %d, fxSpread: %d, fxSpeed: %f\n", settings.fxColor.R, settings.fxColor.G, settings.fxColor.B, settings.fxSize, settings.fxParts, settings.fxSpread, settings.fxSpeed);
        stopFX();
        setupAnimationsCyclon();
        FX.fxRunning = true;
        FX.previousFxNum = 3;
      }
      if(FX.speedChanged) {
        FX.animations2.ChangeAnimationDuration(1, (uint16_t)((SPEED_MAX_DOUBLE - settings.fxSpeed)*1000 + 5));
        FX.speedChanged = false;
      }
      FX.animations2.UpdateAnimations();
      break;
    case 4: //RGB FX
      if(!fxTicker.active()) {
        stopFX();
        fxTicker.attach_ms(1000/FX.fps, sinusRGB);
        FX.fxRunning = true;
        FX.previousFxNum = 4;
      }
      if(fxTicker.active() && FX.previousFxNum != 4) {
        stopFX();
        fxTicker.attach_ms(1000/FX.fps, sinus);
        FX.previousFxNum = 4;
        FX.fxRunning = true;
      }
      break;
    
    default:
      //printf("***Wrong fxNumber\n");
      break;
  }
}

void processPlaylist() {
  if(settings.playlistMode && settings.playlistSize > 0) {
    if((millis() - playlistLastTime) > playlistPeriodMs) {
      if(playlist_counter >= settings.playlistSize){
          while(playlist_counter > 0) {
            playlist_temp--;
            playlist_counter--;
          }
          //printf("playlistCounter: %d\n", playlist_counter);
         // printf("%p, %p\n", playlist_temp, playlist);
        }
        else {
          copyPlaylistSettings(settings, *playlist_temp);
          //printf("cur item: %p\n", playlist_temp);
          playlistLastTime = millis();
          playlist_counter++;
          playlist_temp++;
        }
    }
  }
}

