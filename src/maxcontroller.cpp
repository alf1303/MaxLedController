#include "helper.h"
#include "Ticker.h"

Ticker updateSendTicker;
Ticker fxTicker;

void update() {
  formAnswerInfo(PORT_OUT_UPD);
}

//WiFi Settings
IPAddress apAddress(192, 168, 11, 31);
IPAddress apGateway(192, 168, 11, 31);
IPAddress apSubnet(255, 255, 255, 0);
char ssid[32] = "MaxLedNet";
const char* password = "11223344";


WiFiUDP wifiUDP;

//Pixel Settings

uint8 pixelPin = 2; //ignored for esp8266

//As access point
void connectWiFi_AP() {
  WiFi.enableAP(1);
  bool res_config = WiFi.softAPConfig(apAddress, apGateway, apSubnet);
  bool res_create = WiFi.softAP(ssid, password);
  delay(15);
  printf("Creating Access point...\n");
}

void startUdpServer() {
  bool res = wifiUDP.begin(PORT_IN);
  if(res == 1) 
    printf("Opened UDP socket on port %d\n", PORT_IN);
  else 
    printf("Error opening UDP socket\n");
  delay(15);
  uni = WiFi.softAPIP()[3];
}

//as client
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
   // Open ArtNet port for WIFI
  return state;
}

void readUDP() {
  if(wifiUDP.parsePacket()) {
    printf("udp in\n");
    wifiUDP.read(hData, 5);
    command = hData[3];
    option = hData[4];
    printf("ReadUDP header1: 0: %c, 1: %c, 2: %d, 3: %c, 4: %c, 5: %c\n", (char)hData[0], (char)hData[1], hData[2], (char)hData[3], (char)hData[4]);
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
            printf("***received: netMode: %d, ssidSize: %d, passSize: %d\n", settings.netMode, ssidSize, passwordSize);
            saveNetworkDataToFs(false);
            saveSettingsToFs(false);
            //setReset();
          }
        }

        if(hData[4] == 'P') {
          fxTicker.detach();
          uint8_t low = wifiUDP.read();
          uint8_t high = wifiUDP.read();
          printf("low: %d, high: %d, result: %d\n", low, high, low + (high << 8));
          settings.pixelCount = low + (high << 8);
          settings.endPixel = settings.pixelCount;
          printf("Before Save: pixelCount: %d, endPixel: %d\n", settings.pixelCount, settings.endPixel);
          saveSettingsToFs(false);
          printf("after save\n");
          delete fxData;
          delete fxTemp;
          initFxData();
          printf("end of pixel changing\n");
          //ESP.reset();
          setReset();
        }

      }
      processRequest();
    }
printf("**udpreceive** name: %s, network: %s, password: %s, count: %d, fxBlue: %d\n", settings.name, settings.network, settings.password, settings.pixelCount, settings.fxColor.B);
printf("**udpreceive** fxNum: %d, fxSpeed: %f, fxParts: %d, fxR: %d, fxG: %d, fxB: %d\n", settings.fxNumber, settings.fxSpeed, settings.fxParts, settings.fxColor.R, settings.fxColor.G, settings.fxColor.B);
printf("**udpreceive** fxSpread: %d, fxWidth: %f, fxParams: %d, fxSize: %d\n", settings.fxSpread, settings.fxWidth, settings.fxParams, settings.fxSize);
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  LittleFS.begin();
  strip.Begin();
  //LittleFS.format();
  setRandomSsidName();
  printf("%s\n", ssid);
  initSettings();
  initFxData();
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
  //fxTicker.attach_ms(1000/FX.fps, sinus);
  //printf("0: %d, 0.5: %d, 1.0: %d, 2.0: %d *** 0: %f, 25: %f, 50: %f, 100: %f\n", speedToInt(0), speedToInt(0.5), speedToInt(1), speedToInt(2), speedToDouble(0), speedToDouble(25), speedToDouble(50), speedToDouble(100));
  printf("**setup** name: %s, network: %s, password: %s, count: %d, fxBlue: %d\n", settings.name, settings.network, settings.password, settings.pixelCount, settings.fxColor.B);
  printf("**setup** pixelCount: %d, startPix: %d, endPix: %d, netMode: %d\n",settings.pixelCount, settings.startPixel, settings.endPixel, settings.netMode);
  printf("**setup** fxNum: %d, fxSpeed: %f, fxParts: %d, fxSpread: %d\n",settings.fxNumber, settings.fxSpeed, settings.fxParts, settings.fxSpread);
}

void loop() {
  readUDP();
  processFx();
  outToStrip();
  if(toAnswer) {
    formAnswerInfo(PORT_OUT_UPD);
    toAnswer = false;
  }
  ArduinoOTA.handle();
}

void initFxData() {
  fxData = new RgbColor[settings.pixelCount]();
  fxTemp = new RgbTemp_t[settings.pixelCount];
  attackTemp = new RgbTemp_t[settings.pixelCount];
  clearFxData();
}

void clearFxData() {
  for(int i = 0; i < settings.pixelCount; i++) {
    fxData[i] = black;
    fxTemp[i] = RgbTemp_t(0, 0, 0);
  }
}

void setRandomSsidName() {
 char mac[64];
 String macaddr = WiFi.macAddress().substring(11);
 strcpy(mac, macaddr.c_str());
 strcat(ssid, mac);
}

void processFx() {
  switch(settings.fxNumber) {
    case 0:
      if(FX.fxRunning) {
        fxTicker.detach();
        clearFxData();
        FX.fxRunning = false;
        FX.needRecalculate = true;
        if(FX.previousFxNum != 0) FX.previousFxNum = 0;
        printf("Stopped FX...\n");
      }
      break;
    case 1:
      if(!fxTicker.active()) {
        fxTicker.attach_ms(1000/FX.fps, sinus);
        FX.fxRunning = true;
      }
      if(fxTicker.active() && FX.previousFxNum != 1) {
        fxTicker.detach();
        clearFxData();
        fxTicker.attach_ms(1000/FX.fps, sinus);
        FX.previousFxNum = 1;
        FX.fxRunning = true;
      }
      break;
    case 2:
      if(fxTicker.active()) fxTicker.detach();
      if(FX.previousFxNum != 2) {
        clearFxData();
        setupAnimations();
        FX.previousFxNum = 2;
      }
      animations.UpdateAnimations();
      break;
    
    default:
      printf("***Wrong fxNumber\n");
      break;
  }
}