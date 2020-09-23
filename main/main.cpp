void setup() {

}

void loop() {

}

extern "C" void app_main(void) {
  // esp32 entry
  setup();
  while(1) loop();
}