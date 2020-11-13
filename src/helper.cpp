#include <helper.h>

bool toAnswer = false;
bool isFading = false;
uint8_t fred = 0;
uint8_t fgreen = 0;
uint8_t fblue = 0;
uint8_t ffdim = 0;
int fade_ms = 600;
uint8_t fade_frame_dur = 30;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(200, PIN);
FxController FX = FxController();
RgbColor *fxData;
RgbTemp_t *fxTemp;
settings_t settings = {
    "esp001\0",
    netMode: 1,
    pixelCount: PIXELCOUNT,
    "SolidNet2\0", //ssid for client mode
    "kmdsdobyf3kmdsdobyf3\0", //password for client mode
    playlistSize: 0,
    dimmer: 255,
    color: black,
    fxColor: blue,
    strobe: 255,
    fxNumber: 0,
    fxSpeed: 128,
    fxSize: 255,
    fxParts: 1,
    fxFade: 0,
    fxReverse: 0,
    startPixel: 0,
    endPixel: PIXELCOUNT-1
};

settings_t request;

RgbColor black(0, 0, 0);
RgbColor red(255, 0, 0);
RgbColor green(0, 255, 0);
RgbColor blue(0, 0, 255);
RgbColor highliteColor(150, 150, 150);
RgbColor beforeHLColor;
bool _highlite = false;

uint8_t hData[5];
uint8_t hData1[13];
uint8_t hData2[15];
char command;
char option;
int mask;
IPAddress sourceIP;
uint8_t uni = UNI;

void recalculateTemp() {
  for(int i = 0; i < settings.pixelCount; i++) {
    double t = 6.28/(settings.pixelCount/1);
    fxTemp[i] = RgbTemp_t(i*t, i*t, i*t);
  }
  FX.needRecalculate = false;
  printf("%f, %f, %f\n", fxTemp[0].R, fxTemp[1].R, fxTemp[2].R);
}

void sinus() {
  //printf("sinus\n");
  double speed = 0.3*(6.28/FX.fps);
  if(FX.needRecalculate) recalculateTemp();
  for(int i = 0; i < settings.pixelCount; i++) {
    fxTemp[i] = RgbTemp_t(fxTemp[i].R+speed, fxTemp[i].G+speed, fxTemp[i].B+speed);
    RgbTemp_t res(settings.fxColor.R*sin(fxTemp[i].R), settings.fxColor.G*sin(fxTemp[i].B), 100*sin(fxTemp[i].B));
    fxData[i] = RgbColor(res.R > 0 ? res.R : 0, res.G > 0 ? res.G : 0, res.B > 0 ? res.B : 0);
  }
}



void initSettings() {
  if(!LittleFS.exists(MAIN_FILE)) {
    printf("**** main_file not exists, creating...\n");
    saveSettingsToFs(true);
  }
  if(!LittleFS.exists(NAME_FILE)) {
    printf("***namefile not exists, creating...\n");
    saveNameToFs(true);
  }
  if(!LittleFS.exists(SSID_FILE)) {
    printf("*** ssid or pass file not exists, creating...\n");
    saveNetworkDataToFs(true);
  }
    printf("Reading values from mode_file\n");
    loadSettingsFromFs();
    printf("Readed settings\n");
}

void saveSettingsToFs(boolean first) {
  File f;
  if(first) {
    f = LittleFS.open(MAIN_FILE, "w+");
  }
  else {
    f = LittleFS.open(MAIN_FILE, "w");
  }
  if(!f) {
      printf("**** Open mainfile for writing fails\n");
  }
  else {
    uint8_t pixelCountLow = settings.pixelCount;
    uint8_t pixelCountHigh = settings.pixelCount >> 8;
    f.write(settings.netMode); //0
    f.write(pixelCountLow); //1
    f.write(pixelCountHigh); //2
    f.write(settings.playlistSize); //3
    f.write(settings.dimmer); //4
    f.write(settings.color.R); //5
    f.write(settings.color.G); //6
    f.write(settings.color.B); //7
    f.write(settings.fxColor.R); //8
    f.write(settings.fxColor.G); //9
    f.write(settings.fxColor.B); //10
    f.write(settings.strobe); //11
    f.write(settings.fxNumber); //12
    f.write(speedToInt(settings.fxSpeed)); //13
    f.write(settings.fxSize); //14
    f.write(settings.fxParts); //15
    f.write(settings.fxFade); //16
    f.write(settings.fxReverse); //17
    f.write(settings.startPixel); //18
    f.write(settings.startPixel>>8); //19
    f.write(settings.endPixel); //20
    f.write(settings.endPixel>>8); //21
    delay(50);
    f.close();
  }
  printf("Saved Settings\n");
}

void saveNameToFs(bool first) {
  File f_name = LittleFS.open(NAME_FILE, "w");
  if(!f_name) {
    printf("**** Open namefile for writing fails\n");
  }
  else {
    f_name.print(settings.name);
    delay(50);
    f_name.close();
    printf("name saved\n");
  }
}

void saveNetworkDataToFs(boolean first) {
  File ssidfile = LittleFS.open(SSID_FILE, "w");
  if(!ssidfile) {
    printf("**** Open ssidfile for writing fails\n");
  }
  else{
    ssidfile.print(settings.network);
    delay(50);
    ssidfile.close();
  }

  File passfile = LittleFS.open(PASS_FILE, "w");
  if(!passfile) {
    printf("**** Open passfile for writing fails\n");
  }
  else {
    passfile.print(settings.password);
    delay(50);
    passfile.close();
  }
  printf("Network settings saved\n");
}

void loadSettingsFromFs() {
  File f = LittleFS.open(MAIN_FILE, "r");
  uint8_t temp[22];
  f.read(temp, 22);
  f.close();

  File namefile = LittleFS.open(NAME_FILE, "r");
  size_t nSize = namefile.size();
  settings.name = new char[nSize+1];
  namefile.readBytes(settings.name, nSize);
  namefile.close();
  settings.name[nSize] = '\0';

  File ssidfile = LittleFS.open(SSID_FILE, "r");
  size_t sSize = ssidfile.size();
  settings.network = new char[sSize+1];
  ssidfile.readBytes(settings.network, sSize);
  ssidfile.close();
  settings.network[sSize] = '\0';

  File passfile = LittleFS.open(PASS_FILE, "r");
  size_t pSize = passfile.size();
  settings.password = new char[pSize+1];
  passfile.readBytes(settings.password, pSize);
  passfile.close();
  settings.password[pSize] = '\0';

  settings.netMode = temp[0];
  settings.pixelCount = temp[1] + (temp[2]<<8);
  settings.playlistSize = temp[3];
  settings.dimmer = temp[4];
  settings.color = RgbColor(temp[5], temp[6], temp[7]);
  settings.fxColor = RgbColor(temp[8], temp[9], temp[10]);
  settings.strobe = temp[11];
  settings.fxNumber = temp[12];
  settings.fxSpeed = temp[13];
  settings.fxSize = speedToDouble(temp[14]);
  settings.fxParts = temp[15];
  settings.fxFade = temp[16];
  settings.fxReverse = temp[17];
  settings.startPixel = temp[18] + (temp[19]<<8);
  settings.endPixel = temp[20] + (temp[21]<<8);
}

void processGetCommand() {
  switch (option)
  {
  case 'S':
    formAnswerInfo(PORT_OUT);
    toAnswer = true;
    break;
  
  default:
    printf("GET: unknown option\n");
    break;
  }
}

void formAnswerInfo(int portOUT) {
  //fillSettingsFromFs(&temp_set);
  //printf("FormAnswerInfo... uni: %d\n", uni);
  wifiUDP.beginPacket(sourceIP, portOUT);
  wifiUDP.write("CP"); //0-1
  wifiUDP.write(uni);
  wifiUDP.write(VERSION);
  wifiUDP.write(2);
  wifiUDP.write(2);
  wifiUDP.write(0);
  wifiUDP.write(0);
  wifiUDP.write(settings.fxNumber);
  wifiUDP.write(settings.fxNumber);
  wifiUDP.write(speedToInt(settings.fxSpeed));
  wifiUDP.write(speedToInt(settings.fxSpeed));
  wifiUDP.write(settings.color.R);
  wifiUDP.write(settings.color.G);
  wifiUDP.write(settings.color.B);
  wifiUDP.write(settings.color.R);
  wifiUDP.write(settings.color.G);
  wifiUDP.write(settings.color.B); //23
  wifiUDP.write(settings.dimmer); //24
  wifiUDP.write(settings.dimmer); //25
  wifiUDP.write(UNI); //26
  wifiUDP.write(0); //27
  wifiUDP.write(0); //28
  wifiUDP.write(0); //29
  wifiUDP.write(settings.pixelCount); //30
  wifiUDP.write(settings.startPixel); //31
  wifiUDP.write(settings.endPixel); //32
  wifiUDP.write(1); //33
  //NEW
  wifiUDP.write(settings.pixelCount >> 8); //34
  wifiUDP.write(settings.startPixel >> 8); //35
  wifiUDP.write(settings.endPixel >> 8); //36
  wifiUDP.write(settings.fxColor.R); //37
  wifiUDP.write(settings.fxColor.G); //38
  wifiUDP.write(settings.fxColor.B); //39
  wifiUDP.write(settings.strobe); //40
  wifiUDP.write(settings.fxSize); //41
  wifiUDP.write(settings.fxParts); //42
  wifiUDP.write(settings.fxFade); //43
  wifiUDP.write(settings.fxReverse); //44
  wifiUDP.write(settings.netMode); //45
  wifiUDP.write(strlen(settings.name)); //46
  wifiUDP.write(strlen(settings.network)); //47
  wifiUDP.write(strlen(settings.password)); //48
  wifiUDP.write(settings.name);
  wifiUDP.write(settings.network);
  wifiUDP.write(settings.password);
  wifiUDP.endPacket();
}

void processSetCommand() {
  switch(option)
  {
    case 'S':
      setMainSettings();
      break;
    case 'H': 
      setHighliteMode();
      break;
    case 'h':
      unsetHighliteMode();
      break;
    case 'R':
      setReset();
      break;
    case 'F':
      setFormatFS();
      break;

    default:
      printf("SET: unknown option\n");
      break;
  }
}

void setMainSettings() {
  printf("Setting remote settings\n");
  printf("mask: %d\n", mask);
  switch (mask)
  {
  case 1:
    settings.dimmer = request.dimmer;
    break;
  case 2:
    settings.color = request.color;
    break;
  case 4:
    settings.fxSpeed = request.fxSpeed;
    break;
  case 64:
    settings.pixelCount = request.pixelCount;
    settings.startPixel = request.startPixel;
    settings.endPixel = request.endPixel;
    //saveSettingsToFs();
    break;
  case 128:
    isFading = true;
    fred = request.color.R;
    fgreen = request.color.G;
    fblue = request.color.B;
    ffdim = request.dimmer;
    break;
  case 129:
    settings.fxColor = request.fxColor;
    settings.fxFade = request.fxFade;
    settings.fxNumber = request.fxNumber;
    settings.fxParts = request.fxParts;
    settings.fxReverse = request.fxReverse;
    settings.fxSize = request.fxSize;
    settings.fxSpeed = request.fxSpeed;
    break;
  case 255:
    saveSettingsToFs(false);
    break;
  default:
    printf("**** Unknown mask\n");
    break;
  }
    formAnswerInfo(PORT_OUT_UPD);
}

void setHighliteMode() {
  beforeHLColor = strip.GetPixelColor(1);
  setStaticColor(highliteColor);
  _highlite = true;
}

void unsetHighliteMode() {
  _highlite = false;
  setStaticColor(beforeHLColor);
}

void setReset() {
  ESP.restart();
}

void setFormatFS() {
  LittleFS.format();
  ESP.restart();
}

void processRequest() {
  if(command == 'S') {
    processSetCommand();
  }
  if(command == 'G') {
    processGetCommand();
  }
}

void setStaticColor(RgbColor color) {
  for(int i = 0; i < settings.pixelCount; i++) {
    if(i < settings.startPixel || i > settings.endPixel) {
      strip.SetPixelColor(i, black);
    }
    else {
      //strip.SetPixelColor(i, color);
      setPixelColor(i, color);
    }
  }
  showStrip();
}

void setStaticColorDimmed(uint8_t dimmer, RgbColor col) {
   float koeff = dimmer*1.0/255;
   RgbColor tmp_color(col.R*koeff, col.G*koeff, col.B*koeff);
   setStaticColor(tmp_color);
 }

 void setStaticColorDimmedFaded() {
   double temp_red = settings.color.R;
   double temp_green = settings.color.G;
   double temp_blue = settings.color.B;
   double temp_dim = settings.dimmer;
   double r_step = (fred - temp_red)/(fade_ms/fade_frame_dur);
   double g_step = (fgreen - temp_green)/(fade_ms/fade_frame_dur);
   double b_step = (fblue - temp_blue)/(fade_ms/fade_frame_dur);
   double d_step = (ffdim - temp_dim)/(fade_ms/fade_frame_dur);
   for(int i = 0; i < fade_ms/fade_frame_dur; i++) {
     temp_dim = temp_dim + d_step;
     temp_red = temp_red + r_step;
     temp_green = temp_green + g_step;
     temp_blue = temp_blue + b_step;
     RgbColor tmp_color(temp_red, temp_green, temp_blue);
     setStaticColorDimmed(temp_dim, tmp_color);
     delay(fade_frame_dur);
   }
   settings.dimmer = ffdim;
   settings.color = RgbColor(fred, fgreen, fblue);
   isFading = false;
   formAnswerInfo(PORT_OUT_UPD);
 }

void showStrip() {
  if(!_highlite) {
    strip.Show();
  }
}

void setPixelColor(int i, RgbColor A) {
 // RgbColor A(settings.color.R, settings.color.G, settings.color.B);
  RgbColor B(fxData[i].R, fxData[i].G, fxData[i].B);
  //RgbColor ccolor(RgbColor((B.B == 0 ? A.R : 0) + B.R, (B.B == 0 ? A.G : 0) + B.G, (B.B == 0 ? A.B : 0) + B.B));
    RgbColor ccolor(RgbColor(A.R + B.R, A.G + B.G, A.B + B.B));
  //printf("r: %d, g: %d, b: %d\n", ccolor.R, ccolor.G, ccolor.B);
  strip.SetPixelColor(i, ccolor);
}

void outToStrip() {
    if(!isFading) {
      setStaticColorDimmed(settings.dimmer, settings.color);
    }
    else {
      setStaticColorDimmedFaded();
    }
}


void test2() {
  RgbColor blu = RgbColor(0, 0, 50);
  for(int i = 0; i < 3; i++) {
    strip.SetPixelColor(settings.startPixel, blu);
    strip.SetPixelColor(settings.endPixel, blu);
    showStrip();
    delay(100);
    strip.SetPixelColor(settings.startPixel, black);
    strip.SetPixelColor(settings.endPixel, black);
    showStrip();
    delay(200);
  }
}

uint8_t speedToInt(double speed) {
  double result;
  double scale = (SPEED_MAX_INT - SPEED_MIN_INT)/(SPEED_MAX_DOUBLE - SPEED_MIN_DOUBLE);
  if((SPEED_MIN_DOUBLE == 0 && SPEED_MIN_INT == 0) || (SPEED_MIN_DOUBLE != 0 && SPEED_MIN_INT != 0)) {
    result = speed*scale;
  }
  else if(SPEED_MIN_DOUBLE == 0) {
    result = speed*scale + SPEED_MIN_INT;
  }
  else if(SPEED_MIN_INT == 0) {
    result = speed*scale + SPEED_MAX_INT;
  }
  return result;
}

double speedToDouble(uint8_t speed) {
  double result;
  double scale = (SPEED_MAX_DOUBLE - SPEED_MIN_DOUBLE)/(SPEED_MAX_INT - SPEED_MIN_INT);
   if((SPEED_MIN_DOUBLE == 0 && SPEED_MIN_INT == 0) || (SPEED_MIN_DOUBLE != 0 && SPEED_MIN_INT != 0)) {
    result = speed*scale;
  }
  else if(SPEED_MIN_INT == 0) {
    result = speed*scale + SPEED_MIN_DOUBLE;
  }
  else if(SPEED_MIN_DOUBLE == 0) {
    result = speed*scale + SPEED_MAX_DOUBLE;
  }
  return result;
}

//OTA - Flashing over Air
void OTA_Func() {
    ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  }