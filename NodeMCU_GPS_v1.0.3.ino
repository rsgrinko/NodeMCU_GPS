#include "add_lib.h"
#include "config.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
WiFiUDP ntpUDP;////time
NTPClient timeClient(ntpUDP);////time
TinyGPS gps;
SoftwareSerial ss(D5, D4); //d5 прием, d4 передача, преднастройка порта ЖиПиЭс
#include "function.h"
const char* ssid     = STASSID;
const char* password = STAPSK;
unsigned long eptime; //Переменная для времени с начала эпохи

void setup() //инициализация
{
  Serial.begin(115200); //инициализация физического порта
  ss.begin(9600); //инициализация порта ЖиПиЭс

///led
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
clock_prescale_set(clock_div_1);
#endif
pixels.begin(); // инициализация NeoPixel объекта (включая)pixels.setBrightness(10);
pixels.clear(); 
pixels.setBrightness(20);
////led

Serial.print("Connecting to ");
Serial.println(ssid);

WiFi.mode(WIFI_STA); //режим станция
WiFi.begin(ssid, password); //стартуем подключение к точке доступа

while (WiFi.status() != WL_CONNECTED) { // цикл. Ожидание соединения с точкой. пока не будет канала связи - ждемс
delay(100);
Serial.print(".");

if(nap_led >0) { i_led--;} else {i_led++; } //если мы свалили за границы доступного адресного пространства - идем в другую сторону

if(i_led > NUMPIXELS-2) {nap_led = 1;} //меняем направление
pixels.clear(); 
pixels.setPixelColor(i_led, pixels.Color(0, 150, 0));
pixels.show();

if(i_led < 1) { nap_led = 0;}  //меняем направление
}

Serial.println("");
Serial.println("WiFi connected");
Serial.println("IP address: ");
Serial.println(WiFi.localIP());
all_led_off(); //гасим все индикаторы

timeClient.begin();////time
timeClient.update();////time
eptime = timeClient.getEpochTime(); //Получаем время с начала эпохи
}

void loop() //ебашим цикл main
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  

  // каждую секунду проверяем наличие пакета
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available()) //пришел пакет от жипиэс
    {
      char c = ss.read();
      // Serial.write(c); // отладка. переслать nmea данные в физический порт
      if (gps.encode(c)) // удалось раскодировать nmea
        newData = true;
    }
  }

if (newData) // если получены и расшифрованы данные с приемника ЖиПиЭс
{
float flat, flon;
unsigned long age;
gps.f_get_position(&flat, &flon, &age); //получаем, раскодируем, шлем в порт отладки
Serial.print("LAT=");
Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
Serial.print(" LON=");
Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
Serial.print(" SAT=");
Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
Serial.print(" PREC=");
Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
//---------------------------------//
WiFiClient client;
//дальше формируем и отправляем пакет в облако
Serial.println("sending data to server");
client.connect("it-stories.ru", 80);
client.print("GET /gps/gps.php?");
client.print("devid=");
client.print(DEVID);
client.print("&");
client.print("eptime=");////time
client.print(eptime);////time
client.print("&");
client.print("lat=");
client.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
client.print("&");
client.print("lot=");
client.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
client.print("&");
client.print("speed=");
client.print(gps.speed());
client.println(" HTTP/1.1");
client.print("Host: ");
client.println("it-stories.ru");
client.println("Connection: close");
client.println();
client.println();
client.stop();
client.flush();
//----------------------------//
}
//ну а тут шлем в отладочный порт статистику пакетов от приемника
gps.stats(&chars, &sentences, &failed);
Serial.print(" CHARS=");
Serial.print(chars);
Serial.print(" SENTENCES=");
Serial.print(sentences);
Serial.print(" CSUM ERR=");
Serial.println(failed);
if (chars == 0) { Serial.println("** Нет данных, принятых от GPS. Проверьте подключение. **"); } //как понятно, что все плохо. пакетов нет
}
