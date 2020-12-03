#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL343.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL343 accel = Adafruit_ADXL343(12345);

/** The input pins to enable the interrupt on, connected to INT1 and INT2 on the ADXL. */
#define INPUT_PIN_INT2   (11) // Uno = (2)
#define INPUT_PIN_INT1   (2) // Uno = (3)
#define G_PIN            (0) // Uno = (3)
#define Em_PIN           (1) // Uno = (3)
#define C_PIN            (21) // Uno = (3)
#define D_PIN            (20) // Uno = (3)
#define SDCARD_CS_PIN    (10)
#define SDCARD_MOSI_PIN  (7)
#define SDCARD_SCK_PIN   (14)

bool chord;

// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;
AudioPlaySdWav           playSdWav2;
AudioPlaySdWav           playSdWav3;
AudioPlaySdWav           playSdWav4;


AudioMixer4              mixer1;         //xy=460,59
AudioOutputI2S           i2s1;           //xy=597,48
AudioConnection          patchCord1(playSdWav3, 0, mixer1, 2);
AudioConnection          patchCord2(playSdWav2, 0, mixer1, 1);
AudioConnection          patchCord3(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord4(playSdWav4, 0, mixer1, 3);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=438,178

// GUItool: end automatically generated code

/**
 * This struct is used to count the number of times that specific interrutps
 * have been fired by the ADXL and detected on the MCU. They will increment
 * by one for each event associated with the specified interrupt 'bit'.
 */
struct adxl_int_stats {
    uint32_t data_ready;
    uint32_t single_tap;
    uint32_t double_tap;
    uint32_t activity;
    uint32_t inactivity;
    uint32_t freefall;
    uint32_t watermark;
    uint32_t overrun;
    uint32_t total;
};

/** Global stats block, incremented inside the interrupt handler(s). */
struct adxl_int_stats g_int_stats = { 0 };

/** Global counter to track the numbers of unused interrupts fired. */
uint32_t g_ints_fired = 0;

/** Global variable to determine which interrupt(s) are enabled on the ADXL343. */
int_config g_int_config_enabled = { 0 };

/** Global variables to determine which INT pin interrupt(s) are mapped to on the ADXL343. */
int_config g_int_config_map = { 0 };
sensors_event_t event;
  
/** Interrupt service routine for INT1 events. */


/** Interrupt service routine for INT2 events. */
void int1_isr(void)
{
    /* By default, this sketch routes the DATA_READY interrupt to INT2. */
    delayMicroseconds(200);
    accel.getEvent(&event);
    //float x,y,z;
//    Serial.print(event.acceleration.x);
//    Serial.print(" + ");
//    Serial.print(event.acceleration.y);
//    Serial.print(" + ");
//    Serial.println(event.acceleration.z);
//    Serial.println(accel.checkInterrupts());
    accel.checkInterrupts();
//    if(event.acceleration.z>0){
      Serial.println("Chord Struck");
      chord=true;
//    }
}



/** Configures the HW interrupts on the ADXL343 and the target MCU. */
void config_interrupts(void)
{
  /* NOTE: Once an interrupt fires on the ADXL you can read a register
   *  to know the source of the interrupt, but since this would likely
   *  happen in the 'interrupt context' performing an I2C read is a bad
   *  idea since it will block the device from handling other interrupts
   *  in a timely manner.
   *
   *  The best approach is to try to make use of only two interrupts on
   *  two different interrupt pins, so that when an interrupt fires, based
   *  on the 'isr' function that is called, you already know the int source.
   */

  /* Attach interrupt inputs on the MCU. */
  pinMode(INPUT_PIN_INT1, INPUT);
  //pinMode(INPUT_PIN_INT2, INPUT);
  attachInterrupt(digitalPinToInterrupt(INPUT_PIN_INT1), int1_isr, RISING);
  //attachInterrupt(digitalPinToInterrupt(INPUT_PIN_INT2), int2_isr, RISING);


  accel.disableInterrupt();
  /* Enable interrupts on the accelerometer. */
 

  /* Map specific interrupts to one of the two INT pins. */
  g_int_config_map.bits.overrun    = ADXL343_INT2;
  g_int_config_map.bits.watermark  = ADXL343_INT2;
  g_int_config_map.bits.freefall   = ADXL343_INT2;
  g_int_config_map.bits.inactivity = ADXL343_INT2;
  g_int_config_map.bits.activity   = ADXL343_INT2;
  g_int_config_map.bits.double_tap = ADXL343_INT2;
  g_int_config_map.bits.single_tap = ADXL343_INT1;
  g_int_config_map.bits.data_ready = ADXL343_INT2;
  accel.mapInterrupts(g_int_config_map);
  
  accel.enableAxes(true,false,false,true);      // Suppress x,y,z
  accel.setTHRESHOLD(90); //3g
  accel.setLatency(4);
  accel.setDuration(200);
  
  g_int_config_enabled.bits.overrun    = false;    /* Set the INT1 */
  g_int_config_enabled.bits.watermark  = false;
  g_int_config_enabled.bits.freefall   = false;
  g_int_config_enabled.bits.inactivity = false;
  g_int_config_enabled.bits.activity   = false;
  g_int_config_enabled.bits.double_tap = false;
  g_int_config_enabled.bits.single_tap = true;
  g_int_config_enabled.bits.data_ready = false;    /* Set to INT2 */
  accel.enableInterrupts(g_int_config_enabled);
}

void setup(void)
{
  Serial.begin(9600);
//  while (!Serial);
  AudioMemory(10);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);
  mixer1.gain(2, 0.5);
  mixer1.gain(3, 0.5);
  
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  Serial.println("ADXL343 Interrupt Tester"); Serial.println("");
  pinMode(G_PIN,INPUT_PULLUP);
  pinMode(Em_PIN,INPUT_PULLUP);
  pinMode(C_PIN,INPUT_PULLUP);
  pinMode(D_PIN,INPUT_PULLUP);
  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL343 ... check your connections */
    Serial.println("Ooops, no ADXL343 detected ... Check your wiring!");
    while(1);
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL343_RANGE_16_G);
  // displaySetRange(ADXL343_RANGE_8_G);
  // displaySetRange(ADXL343_RANGE_4_G);
  // displaySetRange(ADXL343_RANGE_2_G);
  
  /* Configure the HW interrupts. */
  config_interrupts();

  Serial.println("ADXL343 init complete. Waiting for INT activity.");
  chord = false;
}

void loop(void)
{
  /* Get a new sensor event */
  accel.getEvent(&event);
  //Serial.print(event.acceleration.x);
  //Serial.print(" + ");
  //Serial.print(event.acceleration.y);
  //Serial.print(" + ");
  //Serial.println(event.acceleration.z);
  if(chord){
    if(digitalRead(G_PIN)==LOW){
      Serial.println("G CHORD");
      playSdWav1.play("G_SHORT.WAV");
      delay(10);
    }
    else if(digitalRead(Em_PIN)==LOW){
      Serial.println("Em CHORD");
      playSdWav2.play("Em_SHORT.WAV");
      delay(10);
    }
    else if(digitalRead(C_PIN)==LOW){
      Serial.println("C CHORD");
      playSdWav3.play("C_SHORT.WAV");
      delay(10);
    }
    else if(digitalRead(D_PIN)==LOW){
      Serial.println("D CHORD");
      playSdWav4.play("D_SHORT.WAV");
      delay(10);
    }
    chord=false;
  }
  
//  playSdWav1.play("G.WAV");
//  delay(2000);
//  playSdWav2.play("Em.WAV");
//  delay(2000);
//  playSdWav3.play("C.WAV");
//  delay(2000);
//  playSdWav4.play("D.WAV");
//  delay(2000);
  delay(50);
} 
