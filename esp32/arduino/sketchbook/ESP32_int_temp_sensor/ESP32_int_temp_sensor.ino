
#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();
uint8_t g_phyFuns;

#ifdef __cplusplus
}
#endif

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

uint8_t temp_farenheit;
float temp_celsius;

void loop() {

    temp_farenheit= temprature_sens_read();
    temp_celsius = ( temp_farenheit - 32 ) / 1.8;
    Serial.print("Temp onBoard ");
    Serial.print(temp_farenheit);
    Serial.print("°F ");
    Serial.print(temp_celsius);
    Serial.println("°C");
    delay(1000);
}
