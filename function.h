void all_led_off() { //функция выключения всех сегментов диодной матрицы
for(int i=0; i<NUMPIXELS; i++) { 
pixels.setPixelColor(i, pixels.Color(0, 0, 0));
}
pixels.show();
}
