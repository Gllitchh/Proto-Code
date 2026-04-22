// ============================================================
// GLITCH PROJECT — PANEL ESP32 (sketch_mar23a.ino)
// ============================================================
// This controller is responsible for one thing: displaying content
// on the two chained 64x32 LED matrix panels.
//
// It listens for commands from the Web ESP32 over a serial (UART) connection.
// When it receives a command it either:
//   - Draws a bitmap image the user drew on the web page, OR
//   - Displays one of the three pre-set face expressions
//
// You do not need to touch this file unless you want to:
//   - Add a new face expression  (see the "FACE COMMANDS" section below)
//   - Change the panel brightness (see setup())
//   - Change which serial pins are used (see #define RX and TX below)
// ============================================================

// ESP32-HUB75-MatrixPanel-I2S-DMA drives the LED matrix panels using DMA
// (Direct Memory Access), which means the display updates happen in the
// background without slowing down the main program.
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <GFX_Layer.hpp>

// ============================================================
// PANEL RESOLUTION AND CHAIN LENGTH
// ============================================================
// These define the size of one panel and how many are chained together.
// We have two 64x32 panels chained, giving a total display of 128x32 pixels.
// Do not change these unless you use different panels.
#define PANEL_RES_X  64    // Width of one panel in pixels
#define PANEL_RES_Y  32    // Height of one panel in pixels
#define PANEL_CHAIN   2    // Number of panels daisy-chained together

// ============================================================
// LED MATRIX PANEL PIN DEFINITIONS
// ============================================================
// These define which GPIO pins on the Panel ESP32 connect to which
// signal on the HUB75 panel header. Do not change these unless
// you rewire the panel connections.
#define R1_PIN  13    // Red data — top half of panel
#define G1_PIN  12    // Green data — top half of panel
#define B1_PIN  14    // Blue data — top half of panel

#define R2_PIN  27    // Red data — bottom half of panel
#define G2_PIN  26    // Green data — bottom half of panel
#define B2_PIN  25    // Blue data — bottom half of panel

#define A_PIN   19    // Row address bit 0
#define B_PIN   21    // Row address bit 1
#define C_PIN   22    // Row address bit 2
#define D_PIN   23    // Row address bit 3
#define E_PIN   18    // Row address bit 4 (needed for 32-row panels)

#define CLK_PIN 33    // Clock signal — synchronises data transfer
#define LAT_PIN  5    // Latch signal — commits a row of data to the display
#define OE_PIN   4    // Output enable — controls overall panel brightness gating

// ============================================================
// PANEL 2 OFFSET
// ============================================================
// The two panels are chained so they act as one 128-pixel wide display.
// P2 = 128 is the horizontal pixel offset of the second panel.
// When drawing faces, every coordinate is drawn twice:
//   once at its normal position (left panel, x = 0-63)
//   once at P2 - x (right panel, x = 64-127)
// This mirrors the face symmetrically across both panels.
#define P2 128

// ============================================================
// UART PIN DEFINITIONS
// ============================================================
// These define the serial pins used to receive data from the Web ESP32.
// TX and RX here are from the Panel ESP32's perspective.
// Do not change these unless you rewire the serial connection.
#define TX 17    // GPIO 17 — sends data to Web ESP32 (acknowledgement)
#define RX 16    // GPIO 16 — receives data from Web ESP32 (commands)

// ============================================================
// DISPLAY OBJECT
// ============================================================
// This pointer will hold the display driver object once it is created in setup().
// It is declared here as a pointer (using *) so it can be initialised later.
MatrixPanel_I2S_DMA *dma_display = nullptr;


// ============================================================
// SETUP — RUNS ONCE WHEN THE ESP32 POWERS ON
// ============================================================
void setup(){

  // Start the USB serial monitor for debug output.
  // Open Serial Monitor in Arduino IDE at 115200 baud to see messages.
  Serial.begin(115200);

  // Start Serial2 for receiving commands from the Web ESP32.
  // Uses GPIO 16 (RX) and GPIO 17 (TX) at 115200 baud.
  // Both ESP32s must use the same baud rate or communication will fail.
  Serial2.begin(115200, SERIAL_8N1, RX, TX);

  // Build the panel configuration structure.
  // This tells the library which pins to use and the panel dimensions.
  // The pin list must match the #define values above exactly.
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,    // Panel width in pixels
    PANEL_RES_Y,    // Panel height in pixels
    PANEL_CHAIN,    // Number of chained panels
    // Pin mapping in the order the library expects:
    // R1, G1, B1, R2, G2, B2, A, B, C, D, E(-1 if unused), LAT, OE, CLK
    {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN,
     A_PIN, B_PIN, C_PIN, D_PIN, -1, LAT_PIN, OE_PIN, CLK_PIN}
    // Note: E_PIN is passed as -1 here because this particular library version
    // handles E differently. If your panels show incorrect row addressing,
    // try replacing -1 with E_PIN.
  );

  // Create the display driver object using the configuration above
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  // Allocate memory and start the DMA display engine.
  // If this fails it means the ESP32 ran out of memory — try reducing PANEL_CHAIN.
  if (!dma_display->begin()){
    Serial.println("ERROR: DMA memory allocation failed — display will not work");
  }

  // Set the panel brightness. Range is 0 (off) to 255 (maximum).
  // 96 is a comfortable indoor brightness. Increase for brighter output,
  // decrease to reduce power consumption and heat.
  dma_display->setBrightness8(96);
}


// ============================================================
// LOOP — RUNS CONTINUOUSLY AFTER SETUP
// ============================================================
void loop(){

  // Check if any data has arrived on Serial2 from the Web ESP32
  if (Serial2.available()){

    // Read the incoming message up to the newline character.
    // The Web ESP32 always ends its messages with \n so we know where they end.
    String msg = Serial2.readStringUntil('\n');
    msg.trim();   // Remove any leading/trailing whitespace or carriage returns

    // Print the message length to Serial for debugging.
    // Open the Serial Monitor to see this output.
    Serial.println("MSG LEN: " + String(msg.length()));

    // If the message is empty after trimming, ignore it and wait for the next one
    if (msg.length() == 0) return;

    Serial.println("Received: " + msg);   // Print the full message for debugging

    // ============================================================
    // BITMAP MODE — draws a custom image sent from the draw page
    // ============================================================
    // Messages that start with "bmp|" contain a user-drawn bitmap.
    // The format is: bmp|<2048 characters of '1' and '0'>
    if (msg.startsWith("bmp|")){

      msg.remove(0, 4);   // Strip the "bmp|" prefix — leaves only the 2048-character bitmap data

      // Validate the bitmap is exactly the right length.
      // 64 pixels wide x 32 pixels tall = 2048 pixels total = 2048 characters.
      if (msg.length() != 2048){
        Serial.println("Bad bitmap size — expected 2048 characters, got " + String(msg.length()));
        return;   // Discard the message and wait for the next one
      }

      // Clear the display before drawing the new bitmap
      dma_display->fillScreen(0);

      // Set the pixel colour — currently cyan (R=0, G=255, B=255).
      // Change these values to change the colour of drawn pixels.
      // color565 converts 8-bit R/G/B values into the 16-bit format the display uses.
      uint16_t c = dma_display->color565(0, 255, 255);

      // Loop through every row (y: 0-31) and every column (x: 0-63)
      for (int y = 0; y < 32; y++){
        for (int x = 0; x < 64; x++){

          // Calculate the position of this pixel in the flat 2048-character string.
          // The x axis is reversed (63-x) to correct for the panel's physical orientation.
          int index = y * 64 + (63 - x);

          // If the character at this position is '1', the pixel should be ON
          if (msg[index] == '1'){
            // Draw on the LEFT panel (x = 0-63, y flipped because panel origin is top-left)
            dma_display->drawPixel(63 - x, 31 - y, c);

            // Draw the mirrored pixel on the RIGHT panel (x + 64 shifts it to the second panel)
            dma_display->drawPixel(x + 64, 31 - y, c);
          }
        }
      }

      return;   // Done with bitmap — go back to listening
    }

    // ============================================================
    // FACE COMMANDS — draws a pre-set face expression
    // ============================================================
    // Any message that is not a bitmap is treated as a face command.
    // First clear the screen before drawing any face.
    dma_display->fillScreen(dma_display->color444(0, 0, 0));

    // ---- NORMAL FACE ----
    // Displays a happy/normal expression in green (R=0, G=255, B=0).
    // The coordinates below define the pixel art for the face.
    // To edit the face, add or remove drawPixel(x, y, c) calls.
    // Each call lights up one pixel at position (x, y).
    // The P2-x calls mirror the same pixel onto the right panel.
    if (msg == "s=normal"){
      uint16_t c = dma_display->color565(0, 255, 0);   // Green — change to alter face colour

      // ---- Left eye (left panel) ----
      dma_display->drawPixel(12, 22, c);
      dma_display->drawPixel(12, 23, c);
      dma_display->drawPixel(12, 24, c);
      dma_display->drawPixel(12, 25, c);
      dma_display->drawPixel(12, 26, c);
      dma_display->drawPixel(13, 27, c);
      dma_display->drawPixel(13, 28, c);
      dma_display->drawPixel(14, 29, c);
      dma_display->drawPixel(15, 30, c);
      dma_display->drawPixel(16, 30, c);
      dma_display->drawPixel(17, 31, c);
      dma_display->drawPixel(18, 31, c);
      dma_display->drawPixel(19, 31, c);
      dma_display->drawPixel(20, 31, c);
      dma_display->drawPixel(21, 31, c);
      dma_display->drawPixel(22, 30, c);
      dma_display->drawPixel(23, 30, c);
      dma_display->drawPixel(24, 29, c);
      dma_display->drawPixel(25, 28, c);
      dma_display->drawPixel(25, 27, c);
      dma_display->drawPixel(26, 26, c);
      dma_display->drawPixel(26, 25, c);
      dma_display->drawPixel(26, 24, c);
      dma_display->drawPixel(26, 23, c);
      dma_display->drawPixel(26, 22, c);
      dma_display->drawPixel(25, 21, c);
      dma_display->drawPixel(25, 20, c);
      dma_display->drawPixel(24, 19, c);
      dma_display->drawPixel(23, 18, c);
      dma_display->drawPixel(22, 18, c);
      dma_display->drawPixel(21, 17, c);
      dma_display->drawPixel(20, 17, c);
      dma_display->drawPixel(19, 17, c);
      dma_display->drawPixel(18, 17, c);
      dma_display->drawPixel(17, 17, c);
      dma_display->drawPixel(16, 18, c);
      dma_display->drawPixel(15, 18, c);
      dma_display->drawPixel(14, 19, c);
      dma_display->drawPixel(13, 20, c);
      dma_display->drawPixel(13, 21, c);

      // ---- Mouth (left panel) ----
      dma_display->drawPixel(29, 10, c);
      dma_display->drawPixel(30,  9, c);
      dma_display->drawPixel(30,  8, c);
      dma_display->drawPixel(31,  7, c);
      dma_display->drawPixel(31,  6, c);
      dma_display->drawPixel(32,  5, c);
      dma_display->drawPixel(33,  4, c);
      dma_display->drawPixel(33,  3, c);
      dma_display->drawPixel(34,  2, c);
      dma_display->drawPixel(34,  1, c);
      dma_display->drawPixel(35,  0, c);
      // Mouth curve up
      dma_display->drawPixel(36,  1, c);
      dma_display->drawPixel(37,  2, c);
      dma_display->drawPixel(38,  3, c);
      dma_display->drawPixel(39,  4, c);
      dma_display->drawPixel(40,  5, c);
      dma_display->drawPixel(41,  6, c);
      dma_display->drawPixel(42,  7, c);
      dma_display->drawPixel(43,  8, c);
      dma_display->drawPixel(44,  9, c);
      dma_display->drawPixel(45, 10, c);
      // Mouth curve down
      dma_display->drawPixel(46,  9, c);
      dma_display->drawPixel(46,  8, c);
      dma_display->drawPixel(47,  7, c);
      dma_display->drawPixel(47,  6, c);
      dma_display->drawPixel(48,  5, c);
      dma_display->drawPixel(49,  4, c);
      dma_display->drawPixel(49,  3, c);
      dma_display->drawPixel(50,  2, c);
      dma_display->drawPixel(50,  1, c);
      dma_display->drawPixel(51,  0, c);
      // Mouth curve up again
      dma_display->drawPixel(52,  1, c);
      dma_display->drawPixel(53,  2, c);
      dma_display->drawPixel(54,  3, c);
      dma_display->drawPixel(55,  4, c);
      dma_display->drawPixel(56,  5, c);
      dma_display->drawPixel(57,  6, c);
      dma_display->drawPixel(58,  7, c);
      dma_display->drawPixel(59,  8, c);
      dma_display->drawPixel(60,  9, c);
      dma_display->drawPixel(61, 10, c);
      dma_display->drawPixel(62, 11, c);
      dma_display->drawPixel(63, 12, c);
      // Corner detail
      dma_display->drawPixel(62, 29, c);
      dma_display->drawPixel(63, 30, c);
      dma_display->drawPixel(63, 31, c);

      // ---- Right eye (right panel — mirrored using P2 offset) ----
      // P2 = 128, so P2-12 = 116, which places the pixel on the second panel
      dma_display->drawPixel(P2-12, 22, c);
      dma_display->drawPixel(P2-12, 23, c);
      dma_display->drawPixel(P2-12, 24, c);
      dma_display->drawPixel(P2-12, 25, c);
      dma_display->drawPixel(P2-12, 26, c);
      dma_display->drawPixel(P2-13, 27, c);
      dma_display->drawPixel(P2-13, 28, c);
      dma_display->drawPixel(P2-14, 29, c);
      dma_display->drawPixel(P2-15, 30, c);
      dma_display->drawPixel(P2-16, 30, c);
      dma_display->drawPixel(P2-17, 31, c);
      dma_display->drawPixel(P2-18, 31, c);
      dma_display->drawPixel(P2-19, 31, c);
      dma_display->drawPixel(P2-20, 31, c);
      dma_display->drawPixel(P2-21, 31, c);
      dma_display->drawPixel(P2-22, 30, c);
      dma_display->drawPixel(P2-23, 30, c);
      dma_display->drawPixel(P2-24, 29, c);
      dma_display->drawPixel(P2-25, 28, c);
      dma_display->drawPixel(P2-25, 27, c);
      dma_display->drawPixel(P2-26, 26, c);
      dma_display->drawPixel(P2-26, 25, c);
      dma_display->drawPixel(P2-26, 24, c);
      dma_display->drawPixel(P2-26, 23, c);
      dma_display->drawPixel(P2-26, 22, c);
      dma_display->drawPixel(P2-25, 21, c);
      dma_display->drawPixel(P2-25, 20, c);
      dma_display->drawPixel(P2-24, 19, c);
      dma_display->drawPixel(P2-23, 18, c);
      dma_display->drawPixel(P2-22, 18, c);
      dma_display->drawPixel(P2-21, 17, c);
      dma_display->drawPixel(P2-20, 17, c);
      dma_display->drawPixel(P2-19, 17, c);
      dma_display->drawPixel(P2-18, 17, c);
      dma_display->drawPixel(P2-17, 17, c);
      dma_display->drawPixel(P2-16, 18, c);
      dma_display->drawPixel(P2-15, 18, c);
      dma_display->drawPixel(P2-14, 19, c);
      dma_display->drawPixel(P2-13, 20, c);
      dma_display->drawPixel(P2-13, 21, c);

      // ---- Mirrored mouth (right panel) ----
      dma_display->drawPixel(P2-128-29, 10, c);
      dma_display->drawPixel(P2-30,  9, c);
      dma_display->drawPixel(P2-30,  8, c);
      dma_display->drawPixel(P2-31,  7, c);
      dma_display->drawPixel(P2-31,  6, c);
      dma_display->drawPixel(P2-32,  5, c);
      dma_display->drawPixel(P2-33,  4, c);
      dma_display->drawPixel(P2-33,  3, c);
      dma_display->drawPixel(P2-34,  2, c);
      dma_display->drawPixel(P2-34,  1, c);
      dma_display->drawPixel(P2-35,  0, c);
      dma_display->drawPixel(P2-36,  1, c);
      dma_display->drawPixel(P2-37,  2, c);
      dma_display->drawPixel(P2-38,  3, c);
      dma_display->drawPixel(P2-39,  4, c);
      dma_display->drawPixel(P2-40,  5, c);
      dma_display->drawPixel(P2-41,  6, c);
      dma_display->drawPixel(P2-42,  7, c);
      dma_display->drawPixel(P2-43,  8, c);
      dma_display->drawPixel(P2-44,  9, c);
      dma_display->drawPixel(P2-45, 10, c);
      dma_display->drawPixel(P2-46,  9, c);
      dma_display->drawPixel(P2-46,  8, c);
      dma_display->drawPixel(P2-47,  7, c);
      dma_display->drawPixel(P2-47,  6, c);
      dma_display->drawPixel(P2-48,  5, c);
      dma_display->drawPixel(P2-49,  4, c);
      dma_display->drawPixel(P2-49,  3, c);
      dma_display->drawPixel(P2-50,  2, c);
      dma_display->drawPixel(P2-50,  1, c);
      dma_display->drawPixel(P2-51,  0, c);
      dma_display->drawPixel(P2-52,  1, c);
      dma_display->drawPixel(P2-53,  2, c);
      dma_display->drawPixel(P2-54,  3, c);
      dma_display->drawPixel(P2-55,  4, c);
      dma_display->drawPixel(P2-56,  5, c);
      dma_display->drawPixel(P2-57,  6, c);
      dma_display->drawPixel(P2-58,  7, c);
      dma_display->drawPixel(P2-59,  8, c);
      dma_display->drawPixel(P2-60,  9, c);
      dma_display->drawPixel(P2-61, 10, c);
      dma_display->drawPixel(P2-62, 11, c);
      dma_display->drawPixel(P2-63, 12, c);
      dma_display->drawPixel(P2-62, 29, c);
      dma_display->drawPixel(P2-63, 30, c);
      dma_display->drawPixel(P2-63, 31, c);
    }

    // ---- BROKEN FACE ----
    // Displays a sad/broken expression in red (R=255, G=0, B=0).
    // Same structure as normal face — edit drawPixel calls to change the expression.
    else if (msg == "s=broken"){
      uint16_t c = dma_display->color565(255, 0, 0);   // Red — change to alter face colour

      // Left eye — downturned expression
      dma_display->drawPixel(12, 22, c);
      dma_display->drawPixel(12, 23, c);
      dma_display->drawPixel(12, 24, c);
      dma_display->drawPixel(12, 25, c);
      dma_display->drawPixel(12, 26, c);
      dma_display->drawPixel(13, 26, c);
      dma_display->drawPixel(13, 25, c);
      dma_display->drawPixel(14, 24, c);
      dma_display->drawPixel(15, 23, c);
      dma_display->drawPixel(16, 23, c);
      dma_display->drawPixel(17, 23, c);
      dma_display->drawPixel(18, 23, c);
      dma_display->drawPixel(19, 23, c);
      dma_display->drawPixel(20, 23, c);
      dma_display->drawPixel(21, 23, c);
      dma_display->drawPixel(22, 24, c);
      dma_display->drawPixel(23, 25, c);
      dma_display->drawPixel(24, 26, c);
      dma_display->drawPixel(25, 28, c);
      dma_display->drawPixel(25, 27, c);
      dma_display->drawPixel(26, 26, c);
      dma_display->drawPixel(26, 25, c);
      dma_display->drawPixel(26, 24, c);
      dma_display->drawPixel(26, 23, c);
      dma_display->drawPixel(26, 22, c);
      dma_display->drawPixel(25, 21, c);
      dma_display->drawPixel(25, 20, c);
      dma_display->drawPixel(24, 19, c);
      dma_display->drawPixel(23, 18, c);
      dma_display->drawPixel(22, 18, c);
      dma_display->drawPixel(21, 17, c);
      dma_display->drawPixel(20, 17, c);
      dma_display->drawPixel(19, 17, c);
      dma_display->drawPixel(18, 17, c);
      dma_display->drawPixel(17, 17, c);
      dma_display->drawPixel(16, 18, c);
      dma_display->drawPixel(15, 18, c);
      dma_display->drawPixel(14, 19, c);
      dma_display->drawPixel(13, 20, c);
      dma_display->drawPixel(13, 21, c);

      // Filled rectangle — a "glitchy" block inside the eye
      // fillRect(x, y, width, height, colour) — change the numbers to resize/reposition it
      dma_display->fillRect(17, 19, 5, 5, c);

      // Damage/crack detail pixels on the left panel
      dma_display->drawPixel(63,  7, c);
      dma_display->drawPixel(62,  7, c);
      dma_display->drawPixel(61,  6, c);
      dma_display->drawPixel(60,  6, c);
      dma_display->drawPixel(59,  6, c);
      dma_display->drawPixel(58,  6, c);
      dma_display->drawPixel(57,  6, c);
      dma_display->drawPixel(61,  7, c);
      dma_display->drawPixel(60,  7, c);
      dma_display->drawPixel(59,  7, c);
      dma_display->drawPixel(58,  7, c);
      dma_display->drawPixel(57,  7, c);
      dma_display->drawPixel(56,  7, c);
      dma_display->drawPixel(55,  7, c);
      dma_display->drawPixel(54,  7, c);
      dma_display->drawPixel(53,  7, c);
      dma_display->drawPixel(52,  7, c);
      dma_display->drawPixel(52,  8, c);
      dma_display->drawPixel(51,  9, c);
      dma_display->drawPixel(50,  9, c);
      dma_display->drawPixel(50, 10, c);
    }

    // ---- ERROR FACE ----
    // Displays a minimal error indicator in blue (R=0, G=0, B=255).
    // Currently just a single pixel — expand with more drawPixel calls to design a full face.
    else if (msg == "s=error"){
      uint16_t c = dma_display->color565(0, 0, 255);   // Blue — change to alter face colour
      dma_display->drawPixel(20, 20, c);   // Single test pixel — add more pixels here to build the face
    }

    // ============================================================
    // TO ADD A NEW FACE:
    // ============================================================
    // 1. Add a new button on the face page in sketch_mar22a.ino (faceHtml)
    //    e.g.  <button onclick="sendPreset('happy')">Happy</button>
    //
    // 2. Add a new else if block here:
    //    else if (msg == "s=happy") {
    //      uint16_t c = dma_display->color565(255, 255, 0);  // Yellow
    //      dma_display->drawPixel(x, y, c);  // Add your pixels here
    //    }
    // ============================================================
  }

  // Wait 30 milliseconds before checking for the next message.
  // This prevents the loop from running thousands of times per second unnecessarily.
  // Increase this number if the display flickers. Decrease for faster response.
  delay(30);
}
