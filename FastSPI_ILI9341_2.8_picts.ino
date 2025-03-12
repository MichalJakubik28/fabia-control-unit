#include <SPI.h>
#include <ILI9341_t3.h>
#include <FlexCAN_T4.h>
#include "defs.h"
#include "lin.h"
#include "iconLib.h"
#include "picture.c"
#include <font_ArialBlack.h>
#include <font_Arial.h>
#include <font_ArialBoldItalic.h>

const int WHEEL_SIZE[3] = {16, 45, 205};
const float INCH = 2.54;
const float KPH_TO_MPS = 3.6;
const int NUMBER_OF_PULSES = 4;
const float SPEED_CONST = KPH_TO_MPS * M_PI * (WHEEL_SIZE[0] * INCH + WHEEL_SIZE[1] * WHEEL_SIZE[2] / 500) / 100 / NUMBER_OF_PULSES;

static speedTable act_speed;
static speedTable cc_speed;
static speedPosition ccPos;
static speedPosition speedPos;
static int lastValue;
static int lastIcon;
static int prevAmbilightIntensity;
static int ambilightIntensity;
static int nCCButton;
static int nFuel;
static int nOldFuel;
static int nRPM;
static int nSpeed;
static int nActiveGear;
static int nDashboardLights;
static float fRatio;
static unsigned int nOdometer;
static unsigned int nOldBatteryVoltage;
static unsigned int nBatteryVoltage;
static unsigned long timePush;
static unsigned long btTimePush;
static unsigned long ccTimePush;
static unsigned long ccResTimePush;
static unsigned long timeChange;
static unsigned long timeLastMotorIns;
static unsigned long timeGearMeasure;
static unsigned long timeDisplaySwitch;
static unsigned long timeLastSpeedPulse;
static bool ccOff;
static bool tpActive;
static bool btActive;
static bool bSpeedChanged;
static bool bGearChanged;
static bool bKey;
static bool bLights;
static bool bReverse;
static bool bShowFuel;
static boolSemaphore bsCCButton;
static boolSemaphore bsFuel;
static boolSemaphore bsImobilizer;
static boolSemaphore bsBrake;
static boolSemaphore bsClutch;
static boolSemaphore bsDriversDoor;
static boolSemaphore bsCoolant;
static boolSemaphore bsCritOil;
static boolSemaphore bsBattery;

static CAN_message_t msg;

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can2;
//FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can3;
Lin lin_tlacidla(Serial4, TX_PIN);
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

#if defined(SPI_HAS_TRANSACTION)
  SPISettings RADIO_SPI(14000000, MSBFIRST, SPI_MODE0);
#endif

void initialize() {
  lastValue = 255;
  ccTimePush = 0;
  ccResTimePush = 0;
  timePush = 0;
  btTimePush = 0;
  timeGearMeasure = 0;
  timeDisplaySwitch = 0;
  timeLastSpeedPulse = 0;
  lastIcon = 0;
  prevAmbilightIntensity = 0;
  nBatteryVoltage = 0;
  nOldBatteryVoltage = 0;
  ambilightIntensity = 113;
  nSpeed = 0;
  nRPM = 0;
  nActiveGear = 0;
  nDashboardLights = 0;
  nFuel = 0;
  nOldFuel = 0;
  fRatio = 0.0;
  ccOff = true;
  tpActive = false;
  btActive = false;
  bSpeedChanged = true;
  bGearChanged = false;
  bLights = false;
  bReverse = false;
  bKey = true;
  bShowFuel = false;
  ccPos = {cckmPos.Y - 5, LEFTB + 35, LEFTB + 50, LEFTB + 65, BLACK};
  speedPos = {kmPos.Y - 44, LEFTB + 10, LEFTB + 55, LEFTB + 100, BLACK};
  act_speed.prev =  0;
  act_speed.prevN1 = 0;
  act_speed.prevN2 = 0;
  act_speed.prevN3 = 0;
}

void splitNumbers(speedTable *st) {
  if (st->act >= 100) {
    st->actN1 = st->act / 100;
    st->actN2 = (st->act - st->actN1 * 100) / 10;
    st->actN3 = st->act % 10;
  } else if (st->act >= 10) {
    st->actN1 = 0;
    st->actN2 = st->act / 10;
    st->actN3 = st->act % 10;
  } else {
    st->actN1 = 0;
    st->actN2 = 0;
    st->actN3 = st->act;
  }
}

void drawIcon(int iconPos, int color) {
  switch (iconPos) {
    case srce: tft.drawBitmap(buttonPos.X, buttonPos.Y, src, 32, 32, color);
      break;
    case volup: tft.drawBitmap(buttonPos.X, buttonPos.Y, vol_up, 32, 32, color);
      break;
    case voldn: tft.drawBitmap(buttonPos.X, buttonPos.Y, vol_dn, 32, 32, color);
      break;
    case volok: tft.drawBitmap(buttonPos.X, buttonPos.Y, vol_ok, 32, 32, color);
      break;
    case gonext: tft.drawBitmap(buttonPos.X, buttonPos.Y, next, 32, 32, color);
      break;
    case goprev: tft.drawBitmap(buttonPos.X, buttonPos.Y, prev, 32, 32, color);
      break;
    case gomenu: tft.drawBitmap(buttonPos.X, buttonPos.Y, menu, 32, 32, color);
      break;
    case seekup: tft.drawBitmap(buttonPos.X, buttonPos.Y, seek_up, 32, 32, color);
      break;
    case seekdn: tft.drawBitmap(buttonPos.X, buttonPos.Y, seek_dn, 32, 32, color);
      break;
    case seekok: tft.drawBitmap(buttonPos.X, buttonPos.Y, seek_ok, 32, 32, color);
      break;
    case voicec: tft.drawBitmap(buttonPos.X, buttonPos.Y, voice, 32, 32, color);
      break;
    case phonep: tft.drawBitmap(buttonPos.X, buttonPos.Y, phone, 32, 32, color);
      break;
    case assist: tft.drawBitmap(buttonPos.X, buttonPos.Y, assistants, 32, 32, color);
      break;
    case goview: tft.drawBitmap(buttonPos.X, buttonPos.Y, view, 32, 32, color);
      break;
    case ccbtn: tft.drawBitmap(ccIconPos.X, ccIconPos.Y, cc, 32, 32, color);
      break;
    case lowfuel: tft.drawBitmap(fuelPos.X, fuelPos.Y, fuel, 32, 32, color);
      break;
    case lowoil: tft.drawBitmap(oilPos.X, oilPos.Y, oil, 32, 32, color);
      break;
    case battery: tft.drawBitmap(batteryPos.X, batteryPos.Y, batt, 32, 32, color);
      break;
    case gearn: tft.drawBitmap(gearPos.X, gearPos.Y, neutral_base, 32, 32, BLACK);
      break;
    case gear1: tft.drawBitmap(gearPos.X, gearPos.Y, neutral_base, 32, 32, BLACK);
      tft.drawBitmap(gearPos.X, gearPos.Y, gearone, 32, 32, WHITE);
      break;
    case gear2: tft.drawBitmap(gearPos.X, gearPos.Y, neutral_base, 32, 32, BLACK);
      tft.drawBitmap(gearPos.X, gearPos.Y, geartwo, 32, 32, WHITE);
      break;
    case gear3: tft.drawBitmap(gearPos.X, gearPos.Y, neutral_base, 32, 32, BLACK);
      tft.drawBitmap(gearPos.X, gearPos.Y, gearthree, 32, 32, WHITE);
      break;
    case gear4: tft.drawBitmap(gearPos.X, gearPos.Y, neutral_base, 32, 32, BLACK);
      tft.drawBitmap(gearPos.X, gearPos.Y, gearfour, 32, 32, WHITE);
      break;
    case gear5: tft.drawBitmap(gearPos.X, gearPos.Y, neutral_base, 32, 32, BLACK);
      tft.drawBitmap(gearPos.X, gearPos.Y, gearfive, 32, 32, WHITE);
      break;
    case gearr: tft.drawBitmap(gearPos.X, gearPos.Y, neutral_base, 32, 32, BLACK);
      tft.drawBitmap(gearPos.X, gearPos.Y, gearrev, 32, 32, WHITE);
      break;
    case cool: tft.drawBitmap(coolantPos.X, coolantPos.Y, coolant, 32, 32, color);
      break;
    case imobil: tft.drawBitmap(imobilizerPos.X, imobilizerPos.Y, imobilizer, 32, 32, color);
      break;
    case dotlin: tft.drawBitmap(dotline1Pos.X, dotline1Pos.Y, dotline, 240, 6, color);
      tft.drawBitmap(dotline2Pos.X, dotline2Pos.Y, dotline, 240, 6, color);
      break;
    case stging: tft.drawBitmap(batteryPos.X, batteryPos.Y, batt, 32, 32, RED);
      tft.drawBitmap(oilPos.X, oilPos.Y, oil, 32, 32, RED);
      tft.drawBitmap(coolantPos.X, coolantPos.Y, coolant, 32, 32, RED);
      tft.drawBitmap(fuelPos.X, fuelPos.Y, fuel, 32, 32, ORANGE);
      tft.drawBitmap(imobilizerPos.X, imobilizerPos.Y, imobilizer, 32, 32, ORANGE);
      showSpeed();
      break;
    case powerdn: tft.fillScreen(BLACK);
      tft.drawBitmap(dotline1Pos.X, dotline1Pos.Y, dotline, 240, 6, DARK_GRAY);
      tft.drawBitmap(dotline2Pos.X, dotline2Pos.Y, dotline, 240, 6, DARK_GRAY);
      break;
  }
}

void setLastIcon(int newIcon) {
  if ((lastIcon != none) && (lastIcon != newIcon)) {
    drawIcon(lastIcon, BLACK);
  }
  lastIcon = newIcon;
}

void sendMessage(int value, uint8_t nValue) {
  if (lastValue != value) {
    lastValue = value;
    digitalWrite(RADIO_ALT, nValue);
    SPI.beginTransaction(RADIO_SPI);
    digitalWrite(DP_CS, LOW);
    SPI.transfer(potRegisterAddress);
    SPI.transfer(value);
    digitalWrite(DP_CS, HIGH);
    SPI.endTransaction();
  }
  if (value > 0) {
    drawIcon(lastIcon, DARKCYAN);
    timePush = micros();
    btTimePush = timePush;
    tpActive = true;
    btActive = true;
  }
}

void resetSpeed(int nCCType, speedPosition *pos, const speedTable *st) {
  tft.setTextColor(BLACK);
  tft.setCursor(pos->posX0, pos->posY);
  if (st->actN1 != st->prevN1) {
    tft.print(st->prevN1);
  }
  tft.setCursor(pos->posX1, pos->posY);
  if (st->actN2 != st->prevN2) {
    tft.print(st->prevN2);
  }
  tft.setCursor(pos->posX2, pos->posY);
  if (st->actN3 != st->prevN3) {
    tft.print(st->prevN3);
  }
  pos->lastColor = BLACK;
}

void equalSpeeds(speedTable *st) {
  st->prev = st->act;
  st->prevN1 = st->actN1;
  st->prevN2 = st->actN2;
  st->prevN3 = st->actN3;
}

void printSpeed(int nCCType, int color) {
  speedPosition *pos = nullptr;
  speedTable *st = nullptr;
  if (nCCType != 0) {
     tft.setFont(Arial_20);
      pos = &ccPos;
      st = &cc_speed;
  } else {
     tft.setFont(Arial_60_Bold_Italic);
      pos = &speedPos;
      st = &act_speed;
  }
  splitNumbers(st);
  resetSpeed(nCCType, pos, st);
  tft.setTextColor(color);
  tft.setCursor(pos->posX0, pos->posY);
  if (st->actN1 != 0) {
    tft.print(st->actN1);
  }
  tft.setCursor(pos->posX1, pos->posY);
  if ((st->actN1 != 0) || (st->actN2 != 0)){
    tft.print(st->actN2);
  }
  tft.setCursor(pos->posX2, pos->posY);
  tft.print(st->actN3);
  equalSpeeds(st);
  if (nCCType > 0) {
    pos->lastColor = color;
  }
}

void showSpeed() {
 tft.setFont(ArialBlack_16);
  tft.setCursor(kmPos.X, kmPos.Y);
  tft.setTextColor(WHITE);
  tft.print("km/h");
  printSpeed(0, WHITE);
}

void showChangedItem(boolSemaphore &semaphore, int icon, int color1) {
  if (semaphore.changed) {
    if (semaphore.active) {
      drawIcon(icon, color1);
    } else {
      drawIcon(icon, BLACK);
    }
    semaphore.changed = false;
  }
}

void showFuelLevel(bool bShow) {
  if (bShow) {
    if (nFuel != nOldFuel) {
      tft.fillRect(fuelValPos.X, fuelValPos.Y, 60, 16, BLACK);
      tft.setFont(ArialBlack_14);
      if (bsFuel.active) {
        tft.setTextColor(ORANGE);
      } else {
        tft.setTextColor(WHITE);
      }
      if (nFuel >= 10) {
        tft.setCursor(fuelValPos.X, fuelValPos.Y);
      } else {
        tft.setCursor(fuelValPos.X + 15, fuelValPos.Y);
      }
      tft.print(nFuel);
      tft.print("L");
      nOldFuel = nFuel;
    }
  } else {
      nOldFuel = 0;
  }
}

void showVoltageLevel(bool bShow){
  if (bShow) {
    float nVoltage = roundf(((float)nBatteryVoltage)/2+50)/10;
    float nVoltageOld = roundf(((float)nOldBatteryVoltage)/2+50)/10;
    if (nVoltage != nVoltageOld) {
      tft.fillRect(fuelValPos.X, fuelValPos.Y, 60, 16, BLACK);
      tft.setFont(ArialBlack_14);
      tft.setCursor(voltPos.X, voltPos.Y);
      if (nVoltage < 11.5) {
        tft.setTextColor(RED);
      } else {
        tft.setTextColor(DARK_GREEN);
      }
      tft.print(nVoltage, 1);
      tft.print("V");  
      nOldBatteryVoltage = nBatteryVoltage;
    }
  } else {
    nOldBatteryVoltage = 0;
  }
}

void showCC(bool bShow) {
  uint16_t customColor;
  if (bShow) {
    customColor = DARK_GRAY;
    cc_speed.act = 0;
    splitNumbers(&cc_speed);
    equalSpeeds(&cc_speed);
  } else {
    customColor = BLACK;
    ccOff = true;
  }
  drawIcon(ccbtn, customColor);
  tft.setTextColor(customColor);
  tft.setFont(Arial_20);
  tft.setCursor(ccPos.posX0, ccPos.posY);
  if (cc_speed.prevN1 != 0) {
    tft.print(cc_speed.prevN1);
  }
  tft.setCursor(ccPos.posX1, ccPos.posY);
  if ((cc_speed.prevN1 != 0) || (cc_speed.prevN2 != 0)){
    tft.print(cc_speed.prevN2);
  }
  tft.setCursor(ccPos.posX2, ccPos.posY);
  tft.print(cc_speed.prevN3);
  ccPos.lastColor = customColor;
  bsCCButton.active = false;
  ccResTimePush = 0;
  ccTimePush = 0;
}

void enableCC(bool bEnable) {
  if (bEnable) {
    drawIcon(ccbtn, DARK_GREEN);
    ccResTimePush = micros();
  } else {
    drawIcon(ccbtn, DARK_GRAY);
    tft.setFont(Arial_20);
    printSpeed(true, DARK_GRAY);
    ccResTimePush = 0;
  }
  ccTimePush = 0;
  bsCCButton.active = bEnable;
}

int evaluateGear() {
  int nGear = 0;
  if (nRPM != 0) {
    fRatio = (((float)nRPM)/((float)nSpeed));
    if ((fRatio >= 3.1) && (fRatio <= 3.9)) {
      nGear = 1;
    } else if ((fRatio > 1.9) && (fRatio <= 2.1)) {
      nGear = 2;
    } else if ((fRatio > 1.2) && (fRatio <= 1.5)) {
      nGear = 3;
    } else if ((fRatio > 0.9) && (fRatio <= 1.1)) {
      nGear = 4;
    } else if ((fRatio > 0.7) && (fRatio <= 0.9)) {
      nGear = 5;
    } else if (bReverse) {
      nGear = -1;
    }
  }
  return nGear;
}

void ext_output1(const CAN_message_t &frame) {
  do {
      switch (frame.id) {
      case 0x11: {
        if (bsImobilizer.active != ((frame.buf[0] & 0x10) == 0)) {
          bsImobilizer.changed = true;
        }
        bsImobilizer.active = ((frame.buf[0] & 0x10) == 0);
        break;
      }
      case 0x280: {
        timeLastMotorIns = micros();
        if (bsClutch.active != ((frame.buf[0] & 0x08) == 0)) {
          bsClutch.active = ((frame.buf[0] & 0x08) == 0);
          bsClutch.changed = true;
          bGearChanged = true;
          timeGearMeasure = micros();
        }
        nRPM = frame.buf[3] * 256 + frame.buf[2];
        break;
      }
      case 0x288: {
        timeLastMotorIns = micros();
        if (bsBrake.active != ((frame.buf[2] & 0x03) == 3)) {
          bsBrake.active = ((frame.buf[2] & 0x03) == 3);
          bsBrake.changed = true;
        }
        break;
      }
      case 0x320: {
        if (bsFuel.active != ((frame.buf[2] & 0x80) == 0x80)) {
          bsFuel.active = ((frame.buf[2] & 0x80) == 0x80);
          bsFuel.changed = true;
        }
        nFuel = (frame.buf[2] & 0x3F);
        if (bsCoolant.active != ((frame.buf[0] & 0x80) == 0x80)) {
          bsCoolant.active = ((frame.buf[0] & 0x80) == 0x80);
          bsCoolant.changed = true;
        }
        if (bsCritOil.active != ((frame.buf[0] & 0x08) == 0x08)) {
          bsCritOil.active = ((frame.buf[0] & 0x08) == 0x08);
          bsCritOil.changed = true;
        }
        if (bsDriversDoor.active != ((frame.buf[0] & 0x01) == 1)) {
          bsDriversDoor.active = ((frame.buf[0] & 0x01) == 1);
          bsDriversDoor.changed = true;
        }
        break;
      }
      case 0x38A: {
        nCCButton = frame.buf[1];
        bsCCButton.changed = true;
        break;
      }
      case 0x470: {
        bLights = (frame.buf[2] != 0);
        nDashboardLights = frame.buf[2];
        bReverse = ((frame.buf[0] & 0x20) == 0x20);
        break;
      }
      case 0x520: {
        nOdometer = frame.buf[7] * 65536 + frame.buf[6] * 256 + frame.buf[5];
        break;
      }
      case 0x570: {
        bool bBattOn = ((frame.buf[0] & 0x80) == 0x0) && ((frame.buf[0] & 0x07) == 0x07);
        if (bsBattery.active != bBattOn) {
          bsBattery.active = bBattOn;
          bsBattery.changed = true;
        }
        nBatteryVoltage = frame.buf[2];
        break;
      }
      case 0x5A8: {
        nSpeed = frame.buf[1] * 256 + frame.buf[0];
        act_speed.act = nSpeed / 144;
        if (act_speed.act != act_speed.prev) {
          bSpeedChanged = true;
        }
        break;
      }
      default: {
        break;
      }
      }
  } while (0);
 /* Serial.print("0x");
  Serial.print(frame.id, HEX);
  for (int i = 0; i < frame.len; i++) {
    if (frame.buf[i] < 16) {
      Serial.print(" 0");
    } else {
      Serial.print(" ");
    }
    Serial.print(frame.buf[i], HEX);
  }
  Serial.println();*/
}

void calcSpeed() {
  static unsigned long currentSpeedPulse = micros();
  act_speed.act = (uint8_t)(SPEED_CONST / (currentSpeedPulse - timeLastSpeedPulse) * 1000000);
  timeLastSpeedPulse = currentSpeedPulse;
  if (act_speed.act != act_speed.prev) {
    bSpeedChanged = true;
  }
}

void setup(){
  initialize();
  pinMode(DP_CS, OUTPUT);   
  pinMode(POT_POWER, OUTPUT);   
  pinMode(RADIO_ALT, OUTPUT);
  pinMode(TFT_LED, OUTPUT);
  pinMode(SPEED_PULSE, INPUT_PULLUP);
  attachInterrupt(SPEED_PULSE, calcSpeed, RISING);
  digitalWrite(RADIO_ALT, LOW);
  lin_tlacidla.begin(19200);
  SPI.begin();
  digitalWrite(POT_POWER, HIGH);
  sendMessage(0, LOW);
//  Serial.begin(19200);
//  while (!Serial && millis() < 5000 );
  Can1.begin();
  Can1.setBaudRate(CAN_SPEED_500k, LISTEN_ONLY);
  Can2.begin();
  Can2.setBaudRate(CAN_SPEED_100k, LISTEN_ONLY);
//  Can3.begin();
//  Can3.setBaudRate(CAN_SPEED_500k, TX);
  Can1.setMaxMB(64);
  Can2.setMaxMB(64);
//  Can3.setMaxMB(64);
  Can1.enableMBInterrupts();
  Can2.enableMBInterrupts();
//  Can3.enableMBInterrupts();
  Can1.setMBFilter(REJECT_ALL);
  Can1.setMBFilter(ACCEPT_ALL);
  Can2.setMBFilter(REJECT_ALL);
  Can2.setMBFilter(ACCEPT_ALL);
//  Can3.setMBFilter(REJECT_ALL);
//  Can3.setMBFilter(ACCEPT_ALL);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(BLACK);
  analogWrite(TFT_LED, 60);
  tft.writeRect(LEFTB - 2, TOPB, 225, 310, (uint16_t*)skoda_logo);
  delay(2000);
  tft.fillScreen(BLACK);
  drawIcon(dotlin, DARK_GRAY);
  drawIcon(stging, WHITE);
  delay(2000);
  tft.fillRect(0, 0, 240, 50, BLACK);
  timeChange = micros();
  timeDisplaySwitch = micros();
}

void loop(){
  uint8_t counter[1] = { 0x71 };
  uint8_t linBuffer[64];
  uint8_t linBufferMultiple = 0;
  uint8_t button;
  uint8_t nRet = 0;
  int nLinDelay = 5;
  int nAmbilight = 0;

  if ((timeDisplaySwitch + 5000000) < micros()) {
    bShowFuel = !bShowFuel;
    timeDisplaySwitch = micros();
  }
  if (tpActive && (timePush + 50000) < micros()) {
    sendMessage(0, LOW);
    timePush = 0;
    tpActive = false;
    digitalWrite(RADIO_ALT, LOW);
  }
  if (btActive && (btTimePush + 500000) < micros()) {
    drawIcon(lastIcon, BLACK);
    lastIcon = none;
    btTimePush = 0;
    btActive = false;
  }
  if (!bLights) {
    nAmbilight = 1;
  } else {
    nAmbilight = ((float)nDashboardLights) * 1.13;
    if (!(nAmbilight % 2)) {
      nAmbilight++;
    }
  }
  counter[0] = nAmbilight;
  lin_tlacidla.send(linButtonsLightAddress, counter, 1, 2);
  memset(linBuffer, 0, sizeof(linBuffer));
  button = 0;
  do {
    nLinDelay--;
    nRet = lin_tlacidla.recv(linButtonsActiveAddress, linBuffer, 8, 2);
  } while ((nRet != 255) && (nLinDelay != 0));
  button = linBuffer[1];
  switch (button) {
    case BT_VOLUME:
      linBufferMultiple = linBuffer[3];
      if (linBufferMultiple < 8) {
        setLastIcon(volup);
        sendMessage(40, LOW);  // VOLUME UP
        for (int i = 0; i < linBufferMultiple; i++) {
          sendMessage(40, LOW);  // VOLUME UP
        }
      } else {
        setLastIcon(voldn);
        sendMessage(60, LOW);  // VOLUME DOWN
        for (int i = 0; i < 16 - linBufferMultiple; i++) {
          sendMessage(60, LOW);  // VOLUME UP
        }
      }
      break;
    case BT_SEEK:
      linBufferMultiple = linBuffer[3];
      if (linBufferMultiple < 8) {
        setLastIcon(seekup);
        sendMessage(22, HIGH);  // PRESET/FOLDER DOWN
        for (int i = 0; i < linBufferMultiple; i++) {
          sendMessage(22, HIGH);  // PRESET/FOLDER DOWN
        }
      } else {
        setLastIcon(seekdn);
        sendMessage(30, HIGH);  // PRESET/FOLDER UP
        for (int i = 0; i < 16 - linBufferMultiple; i++) {
          sendMessage(30, HIGH);  // PRESET/FOLDER UP
        }
      }
      break;
    case BT_SOURCE: 
      setLastIcon(srce);
      sendMessage(4, LOW);       // SOURCE/POWER OFF
      break;
    case BT_NEXT:
      setLastIcon(gonext);
      sendMessage(22, LOW);      // SEARCH/NEXT
      break;
    case BT_PREVIOUS: 
      setLastIcon(goprev);
      sendMessage(30, LOW);      // SEARCH/PREVIOUS
      break;
    case BT_VOLUME_PUSH:
      setLastIcon(volok);
      sendMessage(9, LOW);       // MUTE/VOICE CONTROL
      break;
    case BT_PICKUP:
      setLastIcon(voicec);
      sendMessage(9, HIGH);       // PHONE PICKUP
      break;
    case BT_HANGUP:
      setLastIcon(phonep);
      sendMessage(15, HIGH);      // PHONE HANGUP/REJECT
      break;
    case BT_SEEK_PUSH:
      setLastIcon(seekok);
      sendMessage(120, LOW);      // BAND/ESCAPE
      break;
    case BT_MENU:
      setLastIcon(gomenu);
      sendMessage(4, HIGH);       // PHONE/BLUETOOTH MENU
      break;
      case BT_ASSIST:
      setLastIcon(assist);
      sendMessage(4, HIGH);       // PHONE/BLUETOOTH MENU
      break;
    case BT_VIEW:
      setLastIcon(goview);
      sendMessage(4, HIGH);       // PHONE/BLUETOOTH MENU
      break;
  }
  if (timeLastMotorIns + 100000 < micros()) {
    if (bKey) {
      analogWrite(TFT_LED, 5);
      drawIcon(powerdn, BLACK);
      bKey = false;
    }
    return;
  } else {
    if (!bKey) {
      analogWrite(TFT_LED, 60);
      showSpeed();
      bKey = true;
      bsCCButton.changed = true;
      bSpeedChanged = true;
      bsFuel.changed = true;
      bsImobilizer.changed = true;
      bsBrake.changed = true;
      bsClutch.changed = true;
      bsDriversDoor.changed = true;
      bsCoolant.changed = true;
      bsCritOil.changed = true;
      bsBattery.changed = true;
      nOldFuel = 0;
      timeDisplaySwitch = micros();
      bShowFuel = false;
    }
  }
  if (bsCCButton.changed) {
    switch(nCCButton) {
      case 1: 
        if (ccOff) {
          showCC(true);
          ccOff = false;
        }
        break;
      case 2: 
        showCC(false);
        break;
      case 3: 
        if (bsCCButton.active) {
          enableCC(false);
        }
        break;
      case 5:
        if (act_speed.act < 30) {
          cc_speed.act = 0;
          if (bsCCButton.active) {
            enableCC(false);
          }
        }
        if ((bsCCButton.active) || (!bsCCButton.active && (act_speed.act >= 30))) {
          cc_speed.act = act_speed.act;
          if (!bsCCButton.active) {
            enableCC(true);
          }
          printSpeed(true, DARK_GREEN);
        }
        ccResTimePush = 0;
        break;
      case 9:
        if (bsCCButton.active && (act_speed.act >= 30) && ((ccResTimePush + 1000000) < micros())) {
          ccResTimePush = 0;
          cc_speed.act = act_speed.act;
        }
        if (act_speed.act < 30) {
          cc_speed.act = 0;
          if (bsCCButton.active) {
            enableCC(false);
          } else {
            printSpeed(true, DARK_GRAY);
          }
          ccResTimePush = 0;
        }
        if (!bsCCButton.active && (ccResTimePush == 0) && (cc_speed.act == 0)) {
          ccResTimePush = micros();
          break;
        }
        if ((cc_speed.act == 0) && (ccResTimePush > 0) && ((ccResTimePush + 1000000) < micros())) {
          cc_speed.act = act_speed.act;
          ccResTimePush = 0;
        }
        if (!bsCCButton.active && (act_speed.act >= 30) && (cc_speed.act != 0)) {
          enableCC(true);
        }
        if (bsCCButton.active) {
          splitNumbers(&cc_speed);
          printSpeed(true, DARK_GREEN);
        }
        break;
    }
    nCCButton = 0;
    bsCCButton.changed = false;
    ccTimePush = 0;
  }
  if (bSpeedChanged) {
    tft.setFont(Arial_60_Bold_Italic);
    printSpeed(false, WHITE);
    bSpeedChanged = false;
  }
  showFuelLevel(bShowFuel);
  showChangedItem(bsFuel, lowfuel, ORANGE);
  showChangedItem(bsImobilizer, imobil, ORANGE);
  showChangedItem(bsCoolant, cool, ORANGE);
  showChangedItem(bsCritOil, lowoil, RED);
  showChangedItem(bsBattery, battery, RED);
  showVoltageLevel(!bShowFuel);
  if (bLights) {
    analogWrite(TFT_LED, (nDashboardLights + 20)/2);
  } else {
    analogWrite(TFT_LED, 60);
  }
  if (bGearChanged) {
    if (bsClutch.active && !bReverse) {
      nActiveGear = 0;
    } else {
          nActiveGear = evaluateGear();
    }
    switch (nActiveGear) {
      case 0: drawIcon(gearn, WHITE);
        break;
      case 1: drawIcon(gear1, WHITE);
        break;
      case 2: drawIcon(gear2, WHITE);
        break;
      case 3: drawIcon(gear3, WHITE);
        break;
      case 4: drawIcon(gear4, WHITE);
        break;
      case 5: drawIcon(gear5, WHITE);
        break;
      case -1: drawIcon(gearr, WHITE);
        break;
    }
    bGearChanged = false;
  }
  do {
    if (bsCCButton.active) {
      if (bsClutch.changed && bsClutch.active) {
        enableCC(false);
        bsClutch.changed = false;
      }
      if (bsBrake.changed && bsBrake.active) {
        enableCC(false);
        bsBrake.changed = false;
      }
      if (act_speed.act >= (cc_speed.act + 10)) {
        if (ccTimePush == 0) {
          ccTimePush = micros();
          break;
        }
        if ((ccTimePush + 600000000) < micros()) {
          enableCC(false);
        }
      }
    }
  } while (0);
}
