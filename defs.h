#define color565(r, g, b) ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

// Color definitions
#define BLACK           color565(0, 0, 0)
#define DARK_GREEN      color565(0, 127, 0)
#define DARKCYAN        color565(0, 127, 120)
#define WHITE           color565(255, 255, 255)
#define DARK_GRAY       color565(124, 124, 124)
#define EXTRA_GRAY      color565(64, 64, 64)
#define RED             color565(255, 0, 0)
#define YELLOW          color565(255, 255, 0)
#define ORANGE          color565(255, 100, 0)
#define CYAN            color565(0, 255, 255)

// Define teensy pins for modules
// oled ssd1351
#define TFT_LED      8
#define TFT_DC       9
#define TFT_CS      10
#define TFT_RST    255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12

// digital potentiometer control
#define RX_PIN     16
#define TX_PIN     17
#define DP_CS      5
#define POT_POWER  14
// alternate set of commands for Pioneer
#define RADIO_ALT  6
// input for light intensity

#define SPEED_PULSE 7

// icon definition
#define none     0
#define srce     1
#define volup    2
#define voldn    3
#define volok    4
#define gonext   5
#define goprev   6
#define gomenu   7
#define seekup   8
#define seekdn   9
#define seekok  10
#define voicec  11
#define phonep  12
#define assist  13
#define goview  14
#define hndbrk  15
#define lowfuel 16
#define lowoil  17
#define battery 18
#define gearn   19
#define gear1   20
#define gear2   21
#define gear3   22
#define gear4   23
#define gear5   24
#define gearr   25
#define cool    26
#define imobil  27
#define dotlin  28
#define powerdn 29
#define ccbtn   30
#define stging  31

#define NUM_TX_MAILBOXES 2
#define NUM_RX_MAILBOXES 8

#define CAN_SPEED_500k 500000
#define CAN_SPEED_100k 100000

const int BT_NONE = 0x00;
const int BT_SEEK = 0x06;
const int BT_SEEK_PUSH = 0x07;
const int BT_MENU = 0x08;
const int BT_VOLUME = 0x12;
const int BT_VOLUME_PUSH = 0x13;
const int BT_SOURCE = 0x14;
const int BT_NEXT = 0x15;
const int BT_PREVIOUS = 0x16;
const int BT_PICKUP = 0x19;
const int BT_HANGUP = 0x1C;
const int BT_ASSIST = 0x0C;
const int BT_VIEW = 0x23;

const int linButtonsLightAddress = 0x0D;
const int linButtonsActiveAddress = 0x0E;
const int potRegisterAddress = 0x11;
const float treshold = 600;
const float maximum = 1024;
const float scale = 55;
const float minimum = 3;


typedef struct {
  int X = 0;
  int Y = 0;
} pos;

#define TOPB 12
#define LEFTB 15

const pos kmPos{LEFTB + 155, TOPB + 170};
const pos speedNumPos{LEFTB + 45, TOPB + 163};
const pos cckmPos{LEFTB + 105, TOPB + 284};
//---------------------------------------------
const pos batteryPos{LEFTB + 9, TOPB + 6};
const pos oilPos{LEFTB + 50, TOPB + 6};
const pos coolantPos{LEFTB + 91, TOPB + 3};
//const pos handbrakePos{LEFTB + 111, TOPB + 6};
const pos fuelPos{LEFTB + 132, TOPB + 3};
const pos imobilizerPos{LEFTB + 173, TOPB + 6};
const pos buttonPos{LEFTB + 185, TOPB + 43};
const pos voltPos{LEFTB + 115, TOPB + 282};
const pos gearPos{LEFTB + 188, TOPB + 272};
const pos ccIconPos{LEFTB, TOPB + 270};
const pos dotline1Pos{0, TOPB + 38};
const pos dotline2Pos{0, TOPB + 263};
const pos fuelValPos{LEFTB + 115, TOPB + 282};
//---------------------------------------------
typedef struct {
  uint8_t act = 0;
  uint8_t prev = 0;
  uint8_t actN1 = 0;
  uint8_t actN2 = 0;
  uint8_t actN3 = 0;
  uint8_t prevN1 = 0;
  uint8_t prevN2 = 0;
  uint8_t prevN3 = 0;
} speedTable;

typedef struct {
  int posY = 0;
  int posX0 = 0;
  int posX1 = 0;
  int posX2 = 0;
  int lastColor = 0;
} speedPosition;

typedef struct {
  bool active = false;
  bool changed = false;
} boolSemaphore;
