/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
/* #include "Arduino.h" */
/* #define LED_BUILTIN 13 */

/* void setup() */
/* { */
/*   // initialize LED digital pin as an output. */
/*   pinMode(LED_BUILTIN, OUTPUT); */
/* } */

/* void loop() */
/* { */
/*   // turn the LED on (HIGH is the voltage level) */
/*   digitalWrite(LED_BUILTIN, HIGH); */

/*   // wait for a second */
/*   delay(1000); */

/*   // turn the LED off by making the voltage LOW */
/*   digitalWrite(LED_BUILTIN, LOW); */

/*    // wait for a second */
/*   delay(1000); */
/* } */

#include "Arduino.h"
#include "mjson.h"  // Sketch -> Add file -> add mjson.h

static int ledOn = 0;  // Current LED status

// Gets called by the RPC engine to send a reply frame
static int sender(const char *frame, int frame_len, void *privdata) {
  return Serial.write(frame, frame_len);
}

static void reportState(void) {
  mjson_printf(sender, NULL,
               "{\"method\":\"Shadow.Report\",\"params\":{\"on\":%s}}",
               ledOn ? "true" : "false");
}

static void shadowDeltaHandler(struct jsonrpc_request *r) {
  int ledOn = 0;
  mjson_get_bool(r->params, r->params_len, "$.on", &ledOn);
  digitalWrite(LED_BUILTIN,
               ledOn);              // Set LED to the "on" value
  reportState();                    // Let shadow know our new state
  jsonrpc_return_success(r, NULL);  // Report success
}

void setup() {
  jsonrpc_init(NULL, NULL);
  jsonrpc_export("Shadow.Delta", shadowDeltaHandler);

  pinMode(LED_BUILTIN, OUTPUT);  // Configure LED pin
  Serial.begin(115200);          // Init serial comms
  reportState();                 // Let shadow know our state
}

static void process_byte(unsigned char ch) {
  static char buf[256];  // Buffer that holds incoming frame
  static size_t len;     // Current frame length

  if (len >= sizeof(buf)) len = 0;  // Handle overflow - just reset
  buf[len++] = ch;                  // Append to the buffer
  if (ch == '\n') {                 // On new line, parse frame
    jsonrpc_process(buf, len, sender, NULL, NULL);
    len = 0;
  }
}

void loop() {
  if (Serial.available() > 0) process_byte(Serial.read());
}
