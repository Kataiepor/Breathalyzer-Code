#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int button{8};
const int pin{A0};    // Analog pin connected to sensor
float sensorVolt{0};
float RS{0};           //  Get the value of RS via in a clear air
float R0{0};           // Get the value of R0 via in Alcohol
float sensorValue{0};
float RL{20};      // RL Resistance in Kilo ohms [need to change]

const float X0 = 50;
const float Y0{0.18};
const float X1{500};
const float Y1{0.012};

const float p1[] = { log10(X0), log10(Y0) };
const float p2[] = { log10(X1), log10(Y1) };
const float slope = (p2[1] - p1[1]) / (p2[0] - p1[0]);
const float coord = p1[1] - p1[0] * slope;

int state{0};

void setup()
{
  lcd.init();       // Initialize the LCD displyay screen
  pinMode(A0, INPUT);
  pinMode(button, INPUT);
  function(100000, 10000);
  function(1000000, 10000);
}

void loop()
{
}

// main program
void function(int calibrate, int endDelay)
{
  while (state == 0 )
  {
    state = digitalRead(button);
  }

  state = 0;
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);          // Go to position column 2 & row 1
  lcd.print("Calibrating...");

  // 4 mins
  for (int i{0} ; i < calibrate ; i++)
  {
    sensorValue = sensorValue + analogRead(pin);
  }

  sensorValue = sensorValue / calibrate;      //get average of reading
  sensorVolt = sensorValue / (1024 * 5.0);
  RS = (5.0 - sensorVolt) / sensorVolt;
  R0 = RS / 50.0;                         //50 is found using interpolation from the graph

  delay(10);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Blow into");
  lcd.setCursor(0, 1);
  lcd.print("Mouthpiece");

  float medRS = readMQ(pin);      // Get mean RS resistance value

  delay(10);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calculating...");

  float concentration = getConcentration(medRS / R0); // Get concentration
  float BAC = map(concentration, 20, 500, 7, 184); //20 to 500 scale is from the MQ-3 datasheet graph
  BAC = BAC / 21000000;

  delay(10);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("%BAC: ");
  lcd.print(BAC);
  delay(endDelay);
  lcd.clear();
  lcd.noBacklight();
}


////////////////////////
/// helper functions //
///////////////////////

// Gets the mean value outputted by the sensor
float readMQ(int MQ_Pin)
{
  float rs{0};

  for (int i{0}; i < 30; i++)
  {
    rs += getResistance(analogRead(MQ_Pin));
    delay(500);
  }

  return rs / 30;
}

// Sensor Resistance in Alcohol
float getResistance(int adc)
{
  return (((float)RL / 1000.0 * (1023 - adc) / adc));
}

// Concentration Calculation
float getConcentration(float ratio)
{
  return pow(10, coord + slope * log(ratio));
}
