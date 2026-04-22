// ============================================================
// GLITCH PROJECT — WEB ESP32
// ============================================================
// This controller does everything the user interacts with:
//   - Hosts a Wi-Fi network the user connects to
//   - Serves the web pages in the browser
//   - Moves the two stepper motors (fins)
//   - Reads temperature from the BMP180 sensor
//   - Sends face and bitmap commands to the Panel ESP32
//
// To modify this file you only need the free Arduino IDE.
// Upload to the ESP32 with a USB cable.
// ============================================================

// These lines bring in pre-written code that handles complex tasks for us.
// You do not need to change these unless you swap out hardware.
#include <WiFi.h>              // Handles Wi-Fi — lets the ESP32 create a network
#include <WebServer.h>         // Handles the web server — serves pages to the browser
#include <Wire.h>              // Handles I2C communication — used to talk to the BMP180 sensor
#include <Adafruit_BMP085.h>   // Handles the BMP180 temperature/pressure sensor specifically
#include <EEPROM.h>            // Handles saving data that survives a power cut (like motor positions)
#include <AccelStepper.h>      // Handles smooth stepper motor movement with acceleration

// ============================================================
// WI-FI ACCESS POINT SETTINGS
// ============================================================
// This is the Wi-Fi network name and password that the ESP32 broadcasts.
// Any phone or laptop can connect to this to access the web interface.
// Change apSSID to rename the network. Change apPassword to change the password.
const char* apSSID     = "ESP32-Page";       // The Wi-Fi network name users will see
const char* apPassword = "Glitch and Rome";  // The Wi-Fi password users must enter

// Create the web server object. 80 is the standard port for web pages (HTTP).
// You do not need to change this number.
WebServer server(80);

// ============================================================
// BMP180 TEMPERATURE SENSOR
// ============================================================
// Creates the sensor object we use to read temperature and pressure.
// The Adafruit_BMP085 library works for both BMP085 and BMP180 sensors.
Adafruit_BMP085 bmp;

// ============================================================
// EEPROM — SAVING MOTOR POSITIONS
// ============================================================
// EEPROM is like a tiny notepad that keeps its contents even when power is off.
// We use it to remember where the motors were last left so they restore on boot.
#define EEPROM_SIZE  8   // Total bytes we reserve in EEPROM (8 bytes = enough for two integers)
#define MOTOR1_ADDR  0   // Motor 1 position is saved starting at byte 0
#define MOTOR2_ADDR  4   // Motor 2 position is saved starting at byte 4 (integers are 4 bytes each)

// These variables hold the current motor positions in degrees while the program is running.
// They are loaded from EEPROM at startup and updated whenever the user moves a motor.
int motor1Position = 0;
int motor2Position = 0;

// ============================================================
// UART — SERIAL COMMUNICATION TO PANEL ESP32
// ============================================================
// The Web ESP32 sends commands to the Panel ESP32 over two wires: TX (transmit) and RX (receive).
// These define which GPIO pins those wires are connected to.
// Do not change these unless you rewire the connection.
#define PANEL_TX 17   // GPIO pin 17 sends data OUT to the Panel ESP32
#define PANEL_RX 16   // GPIO pin 16 receives data IN from the Panel ESP32

// ============================================================
// STEPPER MOTOR PIN DEFINITIONS
// ============================================================
// Each stepper motor needs 4 GPIO pins to control its coils.
// IN1 through IN4 match the labels on the ULN2003 driver board.
// Do not change these unless you rewire the motors to different pins.

// Motor 1 pin assignments
#define IN1_1 26   // Motor 1 coil pin 1 — connected to GPIO 26
#define IN2_1 25   // Motor 1 coil pin 2 — connected to GPIO 25
#define IN3_1 33   // Motor 1 coil pin 3 — connected to GPIO 33
#define IN4_1 32   // Motor 1 coil pin 4 — connected to GPIO 32

// Motor 2 pin assignments
#define IN1_2 13   // Motor 2 coil pin 1 — connected to GPIO 13
#define IN2_2 12   // Motor 2 coil pin 2 — connected to GPIO 12
#define IN3_2 14   // Motor 2 coil pin 3 — connected to GPIO 14
#define IN4_2 27   // Motor 2 coil pin 4 — connected to GPIO 27

// The 28BYJ-48 stepper motor takes 4096 steps to complete one full 360 degree rotation
// when driven in half-step mode. This number is used to convert degrees to steps.
#define STEPS_PER_REV 4096

// Create the two stepper motor objects.
// HALF4WIRE means we drive the motor in half-step mode (smoother and more torque).
// The pin order is IN1, IN3, IN2, IN4 — this specific order is required by AccelStepper
// for the 28BYJ-48 motor to turn in the correct direction.
AccelStepper stepper1(AccelStepper::HALF4WIRE, IN1_1, IN3_1, IN2_1, IN4_1);
AccelStepper stepper2(AccelStepper::HALF4WIRE, IN1_2, IN3_2, IN2_2, IN4_2);


// ============================================================
// SHARED MENU HEADER (HTML stored in flash memory)
// ============================================================
// PROGMEM means this string is stored in the ESP32's flash memory instead of RAM.
// This is important because the ESP32 has limited RAM — storing large HTML strings
// in flash prevents the program from running out of memory.
//
// This block contains the CSS styling, the hamburger menu button, and the
// sidebar navigation that appears on every page. Edit the text between the
// <a href=...> tags to rename the menu links.
const char menuHeader[] PROGMEM = R"rawliteral(
<style>
/* Overall page background and font */
body { margin:0; background-color:#1D1F27; color:#FFFFFF; font-family:Arial; }

/* The bar across the top of every page */
.topbar { background:#111; padding:10px; display:flex; align-items:center; }

/* The three-line hamburger menu icon */
.menu-btn { font-size:26px; cursor:pointer; margin-right:15px; }

/* The slide-out sidebar that appears when the hamburger is clicked */
.sidebar { height:100%; width:0; position:fixed; top:0; left:0; background:#222;
           overflow-x:hidden; transition:0.3s; padding-top:60px; z-index:1; }

/* Individual links inside the sidebar */
.sidebar a { padding:12px 20px; text-decoration:none; font-size:18px; color:white; display:block; }
.sidebar a:hover { background:#00A9E0; }  /* Highlight colour when hovering over a link */

/* The centred content box used on most pages */
.container { width:90%; max-width:600px; margin:auto; padding:20px;
             background:#2A2E3B; border-radius:10px; }

/* Default button style */
button { padding:12px 24px; font-size:18px; background:#00A9E0;
         color:white; border:none; border-radius:5px; margin:5px; }

/* The circular gauge shown on the motor page */
.circle { margin:20px auto; width:150px; height:150px; background:#3A3D47;
          border-radius:50%; border:10px solid #444; position:relative; }
canvas { position:absolute; top:0; left:0; }
</style>

<script>
// Toggles the sidebar open and closed when the hamburger button is clicked.
// You do not need to edit this function.
function toggleMenu(){
  let s = document.getElementById('sidebar');
  s.style.width = (s.style.width == '200px') ? '0' : '200px';
}
</script>

<!-- Sidebar navigation — edit the text between > and </a> to rename links -->
<div id='sidebar' class='sidebar'>
  <a href='/'>Motor Control</a>
  <a href='/temp'>Temperature</a>
  <a href='/face'>Face Panel</a>
  <a href='/draw'>Draw Panel</a>
</div>

<!-- Top bar shown on every page -->
<div class='topbar'>
  <span class='menu-btn' onclick='toggleMenu()'>&#9776;</span>
  <h2 style='margin:0'>ESP32 Control</h2>  <!-- Change this text to rename the title bar -->
</div>
)rawliteral";


// ============================================================
// MOTOR CONTROL PAGE HTML — PART 1
// ============================================================
// This is the first half of the motor control page.
// It contains the DOCTYPE declaration, viewport settings (makes the page
// look correct on phones), the page title, and the body tag.
// The onload="load()" calls the load() JavaScript function as soon as
// the page opens, which fetches and restores the last saved motor positions.
const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<title>Motor Control</title>
</head>
<body onload="load()">
)rawliteral";


// ============================================================
// MOTOR CONTROL PAGE HTML — PART 2 (controls and JavaScript)
// ============================================================
// This is the second half of the motor control page.
// It contains the visible controls (input fields, buttons, gauges)
// and all the JavaScript functions that make them work.
const char html2[] PROGMEM = R"rawliteral(
<script>

// Called when the user clicks "Move Motors".
// Reads the angle values from the two input boxes and sends them
// to the ESP32 via a fetch request to /move.
function moveMotors(){
  let m1 = document.getElementById('motor1').value;  // Get Motor 1 angle from input box
  let m2 = document.getElementById('motor2').value;  // Get Motor 2 angle from input box

  // Send the angles to the ESP32 and update the gauges and response text when done
  fetch(`/move?motor1=${m1}&motor2=${m2}`)
    .then(r => r.text())
    .then(t => {
      document.getElementById('response').innerHTML = t;  // Show the ESP32's response
      draw('c1', m1);  // Redraw Motor 1 gauge
      draw('c2', m2);  // Redraw Motor 2 gauge
    });
}

// Called when the user clicks "Clear EEPROM".
// Resets both motors to zero and clears the saved positions from EEPROM.
function clearEEPROM(){
  fetch('/clear')
    .then(r => r.text())
    .then(t => {
      document.getElementById('response').innerHTML = t;  // Show the response
      document.getElementById('motor1').value = 0;        // Reset Motor 1 input to 0
      document.getElementById('motor2').value = 0;        // Reset Motor 2 input to 0
      draw('c1', 0);   // Reset Motor 1 gauge to 0
      draw('c2', 0);   // Reset Motor 2 gauge to 0
    });
}

// Draws the circular angle gauge on a canvas element.
// id    = the canvas element ID ('c1' for Motor 1, 'c2' for Motor 2)
// val   = the angle in degrees to draw (0 to 360)
function draw(id, val){
  let c   = document.getElementById(id);       // Get the canvas element
  let ctx = c.getContext('2d');                 // Get the drawing context
  let r   = c.width / 2;                       // Calculate the radius from the canvas size

  ctx.clearRect(0, 0, c.width, c.height);       // Clear the canvas before redrawing

  // Draw an arc representing the current angle
  ctx.beginPath();
  ctx.arc(r, r, r - 10, -Math.PI / 2, (val / 360) * 2 * Math.PI - Math.PI / 2);
  ctx.lineWidth    = 10;
  ctx.strokeStyle  = '#00A9E0';  // Change this colour to change the gauge colour
  ctx.stroke();
}

// Called automatically when the page loads (see body onload="load()").
// Fetches the last saved motor positions from the ESP32 and fills
// in the input boxes and gauges so they show the correct current state.
function load(){
  fetch('/getMotorPositions')
    .then(r => r.text())
    .then(t => {
      let p = t.split(',');          // The ESP32 sends "angle1,angle2" — split on the comma
      motor1.value = p[0];           // Put Motor 1 angle in its input box
      motor2.value = p[1];           // Put Motor 2 angle in its input box
      draw('c1', p[0]);              // Draw Motor 1 gauge at saved angle
      draw('c2', p[1]);              // Draw Motor 2 gauge at saved angle
    });
}
</script>

<!-- Visible page content -->
<div class='container'>
  <h1>Stepper Motor Control</h1>

  <!-- Motor angle input boxes — change the label text to rename them -->
  Motor 1 Angle <input type='number' id='motor1'><br><br>
  Motor 2 Angle <input type='number' id='motor2'><br><br>

  <!-- Buttons — change the text between > and </button> to rename them -->
  <button onclick='moveMotors()'>Move Motors</button>
  <button onclick='clearEEPROM()'>Clear EEPROM</button>

  <!-- Circular gauges for each motor -->
  <div class='circle'><canvas id='c1' width='150' height='150'></canvas></div>
  <div class='circle'><canvas id='c2' width='150' height='150'></canvas></div>

  <!-- This div shows response messages from the ESP32 (e.g. "Motors Moving") -->
  <div id='response'></div>
</div>
</body>
</html>
)rawliteral";


// ============================================================
// TEMPERATURE PAGE HTML
// ============================================================
// This page displays live temperature, pressure, and altitude readings
// from the BMP180 sensor. The JavaScript setInterval calls /readtemp
// every 2000 milliseconds (2 seconds) and updates the display automatically.
// Change the 2000 to a different number to change how often it updates.
const char tempHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
</head>
<body>
<h1 style='text-align:center'>Temperature Monitor</h1>

<!-- This div shows the sensor readings. It starts with "Loading..." until the first fetch completes -->
<div id='t' style='text-align:center'>Loading...</div>

<script>
// Every 2000ms (2 seconds), fetch fresh sensor data from /readtemp and display it.
// Change 2000 to make it update faster or slower (value is in milliseconds).
setInterval(() => {
  fetch('/readtemp')
    .then(r => r.text())
    .then(t => document.getElementById('t').innerHTML = t);
}, 2000);
</script>
</body>
</html>
)rawliteral";


// ============================================================
// DRAW PAGE HTML — PIXEL CANVAS EDITOR
// ============================================================
// This page provides a 64x32 pixel canvas the user can draw on.
// Drawing tools: Draw (fills pixels cyan), Eraser (clears pixels), Clear All, Send to ESP32.
// When Send is clicked, the bitmap is serialised to a 2048-character string
// and posted to /sendBitmap which forwards it to the Panel ESP32 over Serial2.
const char drawHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<style>
/* Page background */
body { margin:0; background:#1D1F27; color:white; font-family:Arial; }

/* The black bordered frame that holds the canvas */
#panelFrame {
  position:absolute; top:100px; left:80px;
  width:640px; height:320px;        /* Display size — canvas is scaled up 10x from 64x32 */
  border:3px solid #FFFFFF;
  background:#111;
  display:flex; align-items:center; justify-content:center;
}

/* The actual drawing canvas — 64 pixels wide, 32 pixels tall */
#panel { width:640px; height:320px; image-rendering:pixelated; cursor:crosshair; }

/* Button row position */
#controls { position:absolute; top:460px; left:80px; }

/* Button style */
button { padding:10px 20px; margin-right:10px; font-size:16px; }
</style>
</head>

<body>
<h2 style="position:absolute; top:20px; left:40px;">64x32 Panel Editor</h2>

<!-- The frame and canvas -->
<div id="panelFrame">
  <canvas id="panel" width="64" height="32"></canvas>
</div>

<!-- Control buttons -->
<div id="controls">
  <button onclick="setTool('draw')">Draw</button>        <!-- Switch to draw mode -->
  <button onclick="setTool('erase')">Eraser</button>     <!-- Switch to erase mode -->
  <button onclick="clearPanel()">Clear All</button>      <!-- Clear the entire canvas -->
  <button onclick="sendPanel()">Send to ESP32</button>   <!-- Send bitmap to the LED panels -->
</div>

<script>
// Tracks which tool is currently active: 'draw' or 'erase'
let currentTool = 'draw';

// Changes the active tool. Called by the Draw and Eraser buttons above.
function setTool(tool){ currentTool = tool; }

// Get a reference to the canvas and its 2D drawing context
const canvas = document.getElementById("panel");
const ctx    = canvas.getContext("2d");

// The bitmap array — 32 rows of 64 columns, all starting as 0 (off).
// bitmap[y][x] = 1 means that pixel is ON, 0 means OFF.
let bitmap = Array.from({length: 32}, () => Array(64).fill(0));

// Tracks whether the user is currently holding the mouse button down
let drawing = false;

// Tracks the last pixel position — used for line interpolation between samples
let lastX = null;
let lastY = null;

// ---- GRID DRAWING ----
// Draws a faint grid over the canvas so individual pixels are visible.
// You can change the strokeStyle colour to change the grid colour.
function drawGrid(){
  ctx.strokeStyle = "#333";   // Grid line colour — change this to make the grid more/less visible
  ctx.lineWidth   = 0.05;
  for (let x = 0; x <= 64; x++){   // Vertical lines
    ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, 32); ctx.stroke();
  }
  for (let y = 0; y <= 32; y++){   // Horizontal lines
    ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(64, y); ctx.stroke();
  }
}

// ---- CLEAR PANEL ----
// Wipes the canvas and resets all bitmap values to 0 (all pixels off).
function clearPanel(){
  ctx.clearRect(0, 0, canvas.width, canvas.height);  // Clear the visible canvas
  bitmap  = Array.from({length: 32}, () => Array(64).fill(0));  // Reset the bitmap array
  drawing = false;
  lastX   = null;
  lastY   = null;
  drawGrid();  // Redraw the grid so it reappears after clearing
}

// ---- GET PIXEL POSITION FROM MOUSE/TOUCH ----
// Converts the screen coordinates of a mouse or touch event into
// canvas pixel coordinates (0-63 for X, 0-31 for Y).
// Returns null if the pointer is outside the canvas.
function getPixelPos(e){
  const rect    = canvas.getBoundingClientRect();
  const clientX = e.touches ? e.touches[0].clientX : e.clientX;  // Handle both touch and mouse
  const clientY = e.touches ? e.touches[0].clientY : e.clientY;

  // Check if the pointer is within the canvas bounds
  if (clientX < rect.left || clientX > rect.right ||
      clientY < rect.top  || clientY > rect.bottom) return null;

  // Convert screen position to canvas pixel coordinates
  return {
    x: Math.floor((clientX - rect.left)  / 10),  // Divide by 10 because canvas is scaled 10x
    y: Math.floor((clientY - rect.top)   / 10)
  };
}

// ---- DRAW OR ERASE A SINGLE PIXEL ----
// Sets or clears one pixel at position (x, y) depending on the current tool.
// Also updates the bitmap array to match.
function drawPixel(x, y){
  if (currentTool === 'draw'){
    ctx.fillStyle = "#00FFFF";       // Cyan — change this to change the drawing colour
    ctx.fillRect(x, y, 1, 1);       // Fill one pixel on the canvas
    bitmap[y][x] = 1;               // Mark this pixel as ON in the bitmap array
  } else if (currentTool === 'erase'){
    ctx.clearRect(x, y, 1, 1);      // Clear one pixel on the canvas
    bitmap[y][x] = 0;               // Mark this pixel as OFF in the bitmap array
  }
}

// ---- LINE INTERPOLATION (Bresenham's algorithm) ----
// When the mouse moves quickly, it skips pixel positions.
// This function fills in all pixels between the last recorded position
// and the current one, so there are no gaps in drawn lines.
function drawLine(x0, y0, x1, y1){
  let dx  =  Math.abs(x1 - x0);
  let dy  =  Math.abs(y1 - y0);
  let sx  =  x0 < x1 ? 1 : -1;   // Step direction in X
  let sy  =  y0 < y1 ? 1 : -1;   // Step direction in Y
  let err =  dx - dy;

  while (true){
    drawPixel(x0, y0);                      // Draw the current pixel
    if (x0 === x1 && y0 === y1) break;      // Stop when we reach the destination
    let e2 = 2 * err;
    if (e2 > -dy){ err -= dy; x0 += sx; }
    if (e2 <  dx){ err += dx; y0 += sy; }
  }
}

// ---- MOUSE EVENTS ----
// mousedown: start drawing when the button is pressed
canvas.addEventListener("mousedown", (e) => {
  const pos = getPixelPos(e);
  if (!pos) return;
  drawing = true;
  lastX   = pos.x;
  lastY   = pos.y;
  drawLine(lastX, lastY, lastX, lastY);   // Draw a single dot at the click position
});

// mousemove: continue drawing while the button is held down
canvas.addEventListener("mousemove", (e) => {
  if (!drawing) return;
  const pos = getPixelPos(e);
  if (!pos) return;
  drawLine(lastX, lastY, pos.x, pos.y);   // Draw a line from the last position to the current
  lastX = pos.x;
  lastY = pos.y;
});

// mouseup / mouseleave: stop drawing when the button is released or the cursor leaves
canvas.addEventListener("mouseup",    () => drawing = false);
canvas.addEventListener("mouseleave", () => drawing = false);

// ---- TOUCH EVENTS (for phones and tablets) ----
canvas.addEventListener("touchstart", (e) => {
  const pos = getPixelPos(e);
  if (!pos) return;
  drawing = true;
  lastX   = pos.x;
  lastY   = pos.y;
  drawLine(lastX, lastY, lastX, lastY);
});

canvas.addEventListener("touchmove", (e) => {
  if (!drawing) return;
  e.preventDefault();   // Prevents the page from scrolling while drawing
  const pos = getPixelPos(e);
  if (!pos) return;
  drawLine(lastX, lastY, pos.x, pos.y);
  lastX = pos.x;
  lastY = pos.y;
});

canvas.addEventListener("touchend", () => drawing = false);

// ---- SEND BITMAP TO ESP32 ----
// Serialises the 32x64 bitmap array into a single 2048-character string
// of '1' and '0' characters and POSTs it to /sendBitmap on the ESP32.
// The ESP32 then forwards it to the Panel ESP32 over Serial2.
function sendPanel(){
  let bits = "";

  // Loop through every row (y) and every column (x), building the string
  for (let y = 0; y < 32; y++){
    for (let x = 0; x < 64; x++){
      bits += bitmap[y][x] ? "1" : "0";   // Append '1' if pixel is on, '0' if off
    }
  }

  // POST the string to the ESP32
  fetch("/sendBitmap", {
    method: "POST",
    body:   bits + "\n"   // The newline tells the Panel ESP32 the message has ended
  });
}

// Draw the grid when the page first loads so the canvas is not blank
drawGrid();
</script>
</body>
</html>
)rawliteral";


// ============================================================
// FACE PRESET PAGE HTML
// ============================================================
// This page has three buttons for sending pre-defined face expressions
// to the LED panels. Each button calls sendPreset() with the face name,
// which fetches /preset?mode=<name> and the ESP32 forwards it over Serial2.
// To add more face presets, add a new button and a matching handler in
// the /preset route and in the Panel ESP32 sketch.
const char faceHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
</head>
<body>
<div class='container'>
  <h1>Face Presets</h1>

  <!-- Face buttons — change the text and the preset name in sendPreset() to add/rename faces -->
  <button onclick="sendPreset('normal')">Normal</button>
  <button onclick="sendPreset('broken')">Broken</button>
  <button onclick="sendPreset('error')">Error</button>

  <!-- Shows the ESP32's confirmation message after a preset is sent -->
  <div id='response'></div>
</div>
<script>
// Sends the selected face name to the ESP32.
// 'mode' should match one of the face names handled in the Panel ESP32 sketch (s=normal etc).
function sendPreset(mode){
  fetch(`/preset?mode=${mode}`)
    .then(r => r.text())
    .then(t => document.getElementById('response').innerHTML = t);
}
</script>
</body>
</html>
)rawliteral";


// ============================================================
// SETUP — RUNS ONCE WHEN THE ESP32 POWERS ON
// ============================================================
void setup(){

  // Start the USB serial monitor for debug output.
  // Open the Serial Monitor in Arduino IDE at 115200 baud to see messages.
  Serial.begin(115200);

  // Start Serial2 for communication with the Panel ESP32.
  // This uses GPIO 16 (RX) and GPIO 17 (TX) at 115200 baud.
  // Do not change this unless you rewire the UART connection.
  Serial2.begin(115200, SERIAL_8N1, PANEL_RX, PANEL_TX);

  // Start I2C on GPIO 18 (SDA) and GPIO 5 (SCL) for the BMP180 sensor.
  // Do not change these pins unless you rewire the sensor.
  Wire.begin(18, 5);

  // Initialise the BMP180 sensor. If it is not detected, print an error.
  // Check wiring if you see "BMP180 not detected!" in the Serial Monitor.
  if (!bmp.begin()){
    Serial.println("BMP180 not detected!");
  } else {
    Serial.println("BMP180 initialised");
  }

  // Initialise EEPROM and load the last saved motor positions.
  // If the ESP32 has never been used before, these will default to 0.
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(MOTOR1_ADDR, motor1Position);
  EEPROM.get(MOTOR2_ADDR, motor2Position);

  // Configure Motor 1 speed and acceleration.
  // Increase setMaxSpeed for faster movement. Increase setAcceleration for quicker ramp-up.
  stepper1.setMaxSpeed(500);       // Maximum steps per second — increase to move faster
  stepper1.setAcceleration(100);   // Steps per second squared — increase for quicker start/stop

  // Configure Motor 2 speed and acceleration.
  stepper2.setMaxSpeed(500);
  stepper2.setAcceleration(100);

  // Start the Wi-Fi access point with the SSID and password defined at the top of this file.
  WiFi.softAP(apSSID, apPassword);

  // ---- REGISTER HTTP ROUTES ----
  // Each server.on() call tells the web server what to do when a browser
  // requests a specific URL. The lambda function [] inside each call is
  // the code that runs when that URL is visited.

  // Root page "/" — the motor control page
  // Sends the page in three parts to avoid exceeding memory limits:
  // html (opening tags) + menuHeader (sidebar) + html2 (controls and JS)
  server.on("/", [](){
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);    // We don't know the total size in advance
    server.send(200, "text/html; charset=utf-8", "");   // Send a 200 OK response header
    server.sendContent_P(html);        // Send the opening HTML
    server.sendContent_P(menuHeader);  // Send the shared navigation menu
    server.sendContent_P(html2);       // Send the controls and JavaScript
    server.sendContent("");            // Signal end of response
  });

  // "/temp" — the temperature monitor page
  server.on("/temp", [](){
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html; charset=utf-8", "");
    server.sendContent_P(menuHeader);   // Add the navigation menu
    server.sendContent_P(tempHtml);     // Send the temperature page
    server.sendContent("");
  });

  // "/draw" — the pixel canvas editor page
  server.on("/draw", [](){
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html; charset=utf-8", "");
    server.sendContent_P(menuHeader);   // Add the navigation menu
    server.sendContent_P(drawHtml);     // Send the draw page
    server.sendContent("");
  });

  // "/face" — the face preset selection page
  server.on("/face", [](){
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html; charset=utf-8", "");
    server.sendContent_P(menuHeader);   // Add the navigation menu
    server.sendContent_P(faceHtml);     // Send the face page
    server.sendContent("");
  });

  // "/readtemp" — called by the temperature page every 2 seconds
  // Reads from the BMP180 sensor and returns formatted HTML text
  server.on("/readtemp", [](){
    float c   = bmp.readTemperature();          // Temperature in Celsius
    float f   = c * 9.0 / 5.0 + 32.0;          // Convert to Fahrenheit
    float p   = bmp.readPressure() / 100.0;     // Pressure in hPa (divide by 100 to convert from Pa)
    float alt = bmp.readAltitude();             // Estimated altitude in metres

    // Build the HTML response string with all four readings
    String data =
      "Temperature: " + String(c, 2)   + " C<br>" +
      "Temperature: " + String(f, 2)   + " F<br><br>" +
      "Pressure: "    + String(p, 2)   + " hPa<br>" +
      "Altitude: "    + String(alt, 2) + " m";

    server.send(200, "text/html", data);   // Send the readings back to the browser
  });

  // "/sendBitmap" — called when the user clicks "Send to ESP32" on the draw page
  // Receives the 2048-character bitmap string and forwards it to the Panel ESP32
  server.on("/sendBitmap", HTTP_POST, [](){
    String body = server.arg("plain");       // Read the raw POST body (the bitmap string)
    Serial2.println("bmp|" + body);          // Prepend "bmp|" and send over UART to Panel ESP32
    server.send(200, "text/plain", "Bitmap sent");   // Confirm receipt to the browser
  });

  // "/move" — called when the user clicks "Move Motors"
  // Reads motor angles from the URL, converts to steps, and moves both motors
  server.on("/move", [](){

    // If the URL includes a motor1 argument, update Motor 1
    if (server.hasArg("motor1")){
      motor1Position = server.arg("motor1").toInt();   // Read the angle in degrees
      EEPROM.put(MOTOR1_ADDR, motor1Position);          // Save to EEPROM immediately

      // Convert degrees to steps. map() scales -360..360 degrees to -4096..4096 steps.
      long steps = map(motor1Position, -360, 360, -STEPS_PER_REV, STEPS_PER_REV);
      stepper1.moveTo(steps);   // Tell AccelStepper to move to this step position
    }

    // If the URL includes a motor2 argument, update Motor 2
    if (server.hasArg("motor2")){
      motor2Position = server.arg("motor2").toInt();
      EEPROM.put(MOTOR2_ADDR, motor2Position);
      long steps = map(motor2Position, -360, 360, -STEPS_PER_REV, STEPS_PER_REV);
      stepper2.moveTo(steps);
    }

    EEPROM.commit();   // Write the saved values to non-volatile storage
    server.send(200, "text/html", "<h3>Motors Moving</h3>");   // Confirm to the browser
  });

  // "/clear" — called when the user clicks "Clear EEPROM"
  // Resets both motor positions to zero and clears EEPROM
  server.on("/clear", [](){
    stepper1.setCurrentPosition(0);   // Tell AccelStepper Motor 1 is now at position 0
    stepper2.setCurrentPosition(0);   // Tell AccelStepper Motor 2 is now at position 0

    // Overwrite every EEPROM byte with 0
    for (int i = 0; i < EEPROM_SIZE; i++) EEPROM.write(i, 0);
    EEPROM.commit();   // Save the cleared values

    server.send(200, "text/html", "<h3>EEPROM cleared</h3>");
  });

  // "/getMotorPositions" — called by the motor page on load
  // Returns the last saved motor positions as "angle1,angle2"
  server.on("/getMotorPositions", [](){
    server.send(200, "text/plain",
      String(motor1Position) + "," + String(motor2Position));
  });

  // "/preset" — called when the user clicks a face button
  // Sends the face name over Serial2 to the Panel ESP32
  server.on("/preset", [](){
    if (server.hasArg("mode")){
      String mode = server.arg("mode");    // Get the face name (normal, broken, or error)
      Serial2.println("s=" + mode);        // Send "s=normal" etc. to the Panel ESP32
      server.send(200, "text/html", "<h3>Face: " + mode + "</h3>");
    }
  });

  // Start the web server — begins listening for incoming connections
  server.begin();
}


// ============================================================
// LOOP — RUNS CONTINUOUSLY AFTER SETUP
// ============================================================
// This loop keeps three things running at all times:
//   1. The web server — handles any incoming browser request
//   2. Motor 1 — advances one step towards its target if it is not there yet
//   3. Motor 2 — same for Motor 2
//
// Because AccelStepper is non-blocking, the motors and web server run
// together smoothly without one stopping the other.
void loop(){
  server.handleClient();   // Process any pending web request from the browser
  stepper1.run();          // Advance Motor 1 by one step if it has not reached its target
  stepper2.run();          // Advance Motor 2 by one step if it has not reached its target
}
