#include <helper.h>

Ticker fxTicker;
Ticker fxFadeTicker;

FxController FX;
ledsettings_t *playlist;
ledsettings_t *playlist_temp;
uint8_t playlistPeriod = 5;
unsigned long playlistPeriodMs = 5000;
unsigned long playlistLastTime = 0;
uint8_t playlist_counter = 0;

void sinus() {
    //printf("sinus\n");
    double speed = (settings.fxSpeed/4)*((2*PIP)/FX.fps);
    if(FX.needRecalculate) FX.recalculateTemp();
    for(int i = 0; i < settings.pixelCount; i++) {
        FX.fxTemp[i] = RgbTemp_t(FX.fxTemp[i].R+speed, FX.fxTemp[i].G+speed, FX.fxTemp[i].B+speed);
        RgbTemp_t res(settings.fxColor.R*sin(FX.fxTemp[i].R), settings.fxColor.G*sin(FX.fxTemp[i].G), settings.fxColor.B*sin(FX.fxTemp[i].B));
        if(settings.fxAttack) { //RRRRRRRRRRR
        if(FX.fxData[i].R < res.R) {
        FX.attackTemp[i].R = res.R;
        res.R = settings.fxColor.R;
        }
        else {
            if(res.R > FX.attackTemp[i].R) {
            FX.attackTemp[i].R = res.R;
            res.R = settings.fxColor.R;
            }
        } //GGGGGGGGGGGG
        if(FX.fxData[i].G < res.G) {
        FX.attackTemp[i].G = res.G;
        res.G = settings.fxColor.G;
        }
        else {
            if(res.G > FX.attackTemp[i].G) {
            FX.attackTemp[i].G = res.G;
            res.G = settings.fxColor.G;
            }
        } //BBBBBBBBBBBBBBBBBb
        if(FX.fxData[i].B < res.B) {
        FX.attackTemp[i].B = res.B;
        res.B = settings.fxColor.B;
        }
        else {
            if(res.B > FX.attackTemp[i].B) {
            FX.attackTemp[i].B = res.B;
            res.B = settings.fxColor.B;
            }
        }
        }
        FX.fxData[i] = RgbColor(res.R > 0 ? res.R : 0, res.G > 0 ? res.G : 0, res.B > 0 ? res.B : 0);
    }
    }

void sinusRGB() {
    double speed = settings.fxSpeed;
    if(FX.needRecalculate) FX.recalculateTempRGB();
    for(int i = 0; i < settings.pixelCount; i++) {
        FX.rgbData[i] = FX.rgbData[i] + speed;
        double t = ((int)FX.rgbData[i])%360;
        RgbColor col = HslColor(t/360.0, 1.0f, 0.5);
        FX.fxData[i] = col;
    }
}

void moveAnim(const AnimationParam& param) {
    uint16_t nextPixel;
    float progress = NeoEase::SinusoidalInOut(param.progress);
        if (FX.moveDir > 0)
        {
            nextPixel = progress * (settings.pixelCount-1);
        }
        else
        {
            nextPixel = (1.0f - progress) * (settings.pixelCount-1);
        }

    if (FX.lastPixel != nextPixel)
        {
            for (uint16_t i = FX.lastPixel + FX.moveDir; i != nextPixel; i += FX.moveDir)
            {
            if(i >= settings.pixelCount) break;
                FX.fxData[i] = settings.fxColor;
            }
        }
        //printf("np: %d\n", nextPixel);
        FX.fxData[nextPixel] = settings.fxColor;

        FX.lastPixel = nextPixel;

        if(param.state == AnimationState_Completed) {
        FX.moveDir *= -1;
        FX.animations.RestartAnimation(param.index);
        }
}

    void fadeAnim(const AnimationParam& param) {
       if(param.state == AnimationState_Completed) {
          FX.fadeAll();
           if(FX.animations.IsAnimating()) {
            FX.animations.RestartAnimation(param.index);
           }
           if(FX.animations2.IsAnimating()) {
            FX.animations2.RestartAnimation(param.index);
           }
        }
    }

  void animCyclon(const AnimationParam& param) {
    float progress = NeoEase::Linear(param.progress);
    //float progress_step = progress/settings.fxParts;
    uint16_t fxParts_tmp = settings.fxParts == 1 ? settings.pixelCount : settings.fxParts;
    float progress_step = 1.0f/fxParts_tmp;
    size_t progresses_size = (int)ceil(1/progress_step) +1;
    uint16_t indexes[settings.pixelCount];
    float progresses[102];
    uint16_t* ind_t = indexes;
    float* progr_t = progresses;
    uint16_t j = 0;
    uint16_t tmpind = 0;
    RgbColor colo;
    if(settings.fxRndColor) {
        colo = HslColor(random(360)/360.0f, 1.0f, 0.5);
    }
    else {
        colo = settings.fxColor;
    }
    //printf("progr: %f, fxPartsT: %d, progrSize: %d, indexSize: %d, progrStep: %f\n",progress, fxParts_tmp, progresses_size, indexes_size, progress_step);
    if(settings.fxRnd) {
        if(FX.rndShouldGo == -1) {
        FX.rndShouldGo = 0;
        uint16_t count = settings.fxSpread;
        while (count > 0) {
        uint16_t pixel = random(settings.pixelCount);
        *ind_t = pixel;
        ind_t++; 
        tmpind++;
        count--;
        }
        }

    }
    else{
        while (j < progresses_size)  {
        *progr_t = j * progress_step;
        j++;
        progr_t++;
    }
    *progr_t = 1.0f;
    uint16_t i = 0;
    while(i < (progresses_size - 1)) {
        if((progress >= progresses[i]) && (progress <= progresses[i+1])) {
        for(int k = 0; k < settings.pixelCount; k++) {
            if((k%fxParts_tmp) == i) {
            uint16_t cc = k + fxParts_tmp - (i*2+1);
            //printf("k: %d, cc: %d\n", k, cc);
            if(!settings.fxSymm) {
                *ind_t = settings.fxReverse ? (settings.pixelCount - k - 1) : k;
                ind_t++;
                tmpind++;
            }
            else {
                if(cc < settings.pixelCount && cc >= 0) {
                if(settings.fxReverse) {
                    if(cc >= k) {
                        *ind_t = k;
                        ind_t++;
                        tmpind++;
                        *ind_t = cc;
                        ind_t++;
                        tmpind++;
                    }
                }
                else {
                    if(cc <= k) {
                        *ind_t = k;
                        ind_t++;
                        tmpind++;
                        *ind_t = cc;
                        ind_t++;
                        tmpind++;
                    }
                }
            }
            }
            }
        }
        }
        i++;
    }
    }
    ind_t = indexes;
    uint16_t h = 0;
    //printf("tmpind: %d\n", tmpind);
    if(*indexes != FX.prevIndex) {
        while (h < tmpind)
    {
        //printf("h: %d, ind: %d\n",h, *ind_t);
        if(*ind_t < settings.pixelCount && *ind_t >= 0) {
        FX.fxData[*ind_t] = colo;
        }
        ind_t++;
        h++;
    }
    FX.prevIndex = *indexes;
    }
    if(param.state == AnimationState_Completed) {
        FX.rndShouldGo = -1;
        FX.animations2.RestartAnimation(param.index);
        }
    }

void setupAnimations() {
     FX.animations.StartAnimation(0, 7, fadeAnim);
     FX.animations.StartAnimation(1, ((SPEED_MAX_DOUBLE - settings.fxSpeed)*1000+5), moveAnim);
}

void setupAnimationsCyclon() {
    FX.animations2.StartAnimation(0, 15, fadeAnim);
    FX.animations2.StartAnimation(1, ((SPEED_MAX_DOUBLE - settings.fxSpeed)*1000+5), animCyclon);
}

void setRandomSeed() {
  uint32_t seed;
  seed = analogRead(0);
  delay(1);
  for(int shifts = 3; shifts < 31; shifts += 3) {
    seed ^= analogRead(0) << shifts;
  }
  randomSeed(seed);
}

void savePlaylist() {
  printf("save playlist start\n");
  playlistPeriodMs = playlistPeriod*60*1000;
  ledsettings_t *plset = playlist;
  size_t plSize = settings.playlistSize;
  File f = LittleFS.open(PLAYLIST_FILE, "w");
  playlist_counter = 0;
  if(!f) {
    printf("**** fail to open playlistFile for writing\n");
  }
  else {
    f.write(plSize);
    f.write(playlistPeriod);
    for(int i = 0; i < settings.playlistSize; i++) {
      ledsettings_t set = *plset;
      f.write(set.dimmer);
      f.write(set.color.R);
      f.write(set.color.G);
      f.write(set.color.B);
      f.write(set.fxColor.R);
      f.write(set.fxColor.G);
      f.write(set.fxColor.B);
      f.write(set.strobe);
      f.write(set.fxNumber);
      f.write(speedToInt(set.fxSpeed));
      f.write(set.fxSize);
      f.write(set.fxParts);
      f.write(set.fxFade);
      f.write(set.fxParams);
      f.write(set.fxSpread);
      f.write(set.fxWidth);
      plset++;
    }
    delay(10);
    f.close();
    printf("**** playlist saved\n");
  }
}

void loadPlaylist() {
  printf("loading playlist: size: %d\n", settings.playlistSize);
  File f = LittleFS.open(PLAYLIST_FILE, "r");
  printf("playlist file size: %d\n", f.size());
  if(!f) {
    printf("**** Error playlist file, while loading\n");
  }
  else {
    settings.playlistSize = f.read();
    playlist = new ledsettings_t[settings.playlistSize];
    playlist_temp = playlist;
    ledsettings_t *pset = playlist;
    playlistPeriod = f.read();
    playlistPeriodMs = playlistPeriod*60*1000;
    for(int i = 0; i < settings.playlistSize; i++) {
      ledsettings_t set;
      byte temp[16];
      f.read(temp, 16);
      set.dimmer = temp[0];
      set.color = RgbColor(temp[1], temp[2], temp[3]);
      set.fxColor = RgbColor(temp[4], temp[5], temp[6]);
      set.strobe = temp[7];
      set.fxNumber = temp[8];
      set.fxSpeed = speedToDouble(temp[9]);
      set.fxSize = temp[10];
      set.fxParts = temp[11];
      set.fxFade = temp[12];
      set.fxParams = temp[13];
      set.fxSpread = temp[14];
      set.fxWidth = temp[15];
      set.fxReverse = (set.fxParams)&1;
      set.fxAttack = (set.fxParams>>1)&1;
      set.fxSymm = (set.fxParams>>2)&1;
      set.fxRnd = (set.fxParams>>3)&1;
      set.fxRndColor = (set.fxParams>>7)&1;
      *pset = set;
      pset++;
    }
    f.close();
    //printf("**** loaded playlist items\n");
  }
}

void copyPlaylistSettings(settings_t &set, ledsettings_t &pset) {
  set.dimmer = pset.dimmer;
  set.color = pset.color;
  set.fxColor = pset.fxColor;
  set.strobe = pset.strobe;
  set.fxNumber = pset.fxNumber;
  set.fxSpeed = pset.fxSpeed;
  set.fxSize = pset.fxSize;
  set.fxParts = pset.fxParts;
  set.fxFade = pset.fxFade;
  set.fxParams = pset.fxParams;
  set.fxSpread = pset.fxSpread;
  set.fxWidth = pset.fxWidth;
  set.fxReverse = pset.fxReverse;
  set.fxAttack = pset.fxAttack;
  set.fxSymm = pset.fxSymm;
  set.fxRnd = pset.fxRnd;
  set.fxRndColor = pset.fxRndColor;
}