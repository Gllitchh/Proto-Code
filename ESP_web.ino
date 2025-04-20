#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

// Wi-Fi credentials
const char* ssid = "TCC Willis";
const char* password = "7157160546";

// Create instances
ESP8266WebServer server(80);
Adafruit_BMP085 bmp;

// Motor positions (server-side storage)
int motor1Position = 0;
int motor2Position = 0;

// Motor Control HTML Page in PROGMEM
const char html[] PROGMEM = R"rawliteral(
<html>
<head>
  <title>Motor Control</title>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <style>
    body { background-color: #1D1F27; color: #FFFFFF; font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; }
    h1 { color: #00A9E0; margin-top: 50px; }
    .container { width: 90%; max-width: 600px; margin: 0 auto; padding: 20px; background: #2A2E3B; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2); }
    label { font-size: 18px; display: block; margin-top: 10px; text-align: left; color: #A0A0A0; }
    input[type='number'] { width: 90%; padding: 12px; margin-top: 5px; border: none; border-radius: 5px; font-size: 18px; background-color: #3A3D47; color: #FFFFFF; border: 1px solid #444; }
    input[type='number']:focus { background-color: #505460; border-color: #00A9E0; outline: none; }
    input[type='submit'], a.button { background-color: #00A9E0; color: #FFFFFF; padding: 12px 24px; border: none; border-radius: 5px; font-size: 18px; cursor: pointer; text-decoration: none; display: inline-block; margin-top: 20px; }
    input[type='submit']:hover, a.button:hover { background-color: #0087A0; }
    .response { background-color: #2A2E3B; padding: 20px; border-radius: 10px; margin-top: 30px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2); }
    .circle-container { display: flex; flex-wrap: wrap; justify-content: space-around; }
    .circle { margin: 20px; position: relative; width: 150px; height: 150px; background-color: #3A3D47; border-radius: 50%; border: 10px solid #444; }
    .circle > canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; }
    .label { font-size: 16px; color: #A0A0A0; margin-top: 10px; }
  </style>
  <script>
    var motor1Angle = 0;
    var motor2Angle = 0;
    function moveMotors() {
      var motor1 = document.getElementById('motor1').value;
      var motor2 = document.getElementById('motor2').value;
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/move?motor1=' + motor1 + '&motor2=' + motor2, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          document.getElementById('response').innerHTML = xhr.responseText;
          updateCircles(motor1, motor2);
        }
      };
      xhr.send();
      return false;
    }
    function updateCircles(motor1, motor2) {
      motor1Angle = parseInt(motor1);
      motor2Angle = parseInt(motor2);
      drawCircle('motor1Canvas', motor1Angle);
      drawCircle('motor2Canvas', motor2Angle);
    }
    function drawCircle(canvasId, angle) {
      var canvas = document.getElementById(canvasId);
      var ctx = canvas.getContext('2d');
      var radius = canvas.width / 2;
      var startAngle = -0.5 * Math.PI;
      var endAngle = (angle / 360) * 2 * Math.PI + startAngle;
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.beginPath();
      ctx.arc(radius, radius, radius - 10, startAngle, endAngle, angle < 0);
      ctx.lineWidth = 10;
      ctx.strokeStyle = '#00A9E0';
      ctx.stroke();
    }
    function loadMotorPositions() {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', '/getMotorPositions', true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          var positions = xhr.responseText.split(',');
          var motor1Position = positions[0];
          var motor2Position = positions[1];
          document.getElementById('motor1').value = motor1Position;
          document.getElementById('motor2').value = motor2Position;
          updateCircles(motor1Position, motor2Position);
        }
      };
      xhr.send();
    }
    window.onload = loadMotorPositions;
  </script>
</head>
<body>
  <div class='container'>
    <h1>Stepper Motor Control</h1>
    <form onsubmit='return moveMotors()'>
      <label for='motor1'>Motor 1 Angle (-360 to 360): </label>
      <input type='number' id='motor1' name='motor1' min='-360' max='360'><br><br>
      <label for='motor2'>Motor 2 Angle (-360 to 360): </label>
      <input type='number' id='motor2' name='motor2' min='-360' max='360'><br><br>
      <input type='submit' value='Move Motors'>
    </form>
    <a class='button' href='/temp'>Temp</a>
    <div class='circle-container'>
      <div>
        <div class='label'>Motor 1</div>
        <div class='circle'>
          <canvas id='motor1Canvas' width='150' height='150'></canvas>
        </div>
      </div>
      <div>
        <div class='label'>Motor 2</div>
        <div class='circle'>
          <canvas id='motor2Canvas' width='150' height='150'></canvas>
        </div>
      </div>
    </div>
    <div id='response'></div>
  </div>
</body>
</html>
)rawliteral";

// Temperature Page HTML in PROGMEM
const char tempHtml[] PROGMEM = R"rawliteral(
<html>
<head>
  <title>Temperature</title>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <style>
    body { background-color: #1D1F27; color: #FFFFFF; font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; }
    .container { width: 90%; max-width: 600px; margin: 0 auto; padding: 20px; background: #2A2E3B; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2); }
    h1 { color: #00A9E0; margin-top: 50px; }
    #temp { font-size: 24px; margin-top: 20px; color: #A0A0A0; }
    a.button { background-color: #00A9E0; color: #FFFFFF; padding: 12px 24px; border: none; border-radius: 5px; font-size: 18px; text-decoration: none; display: inline-block; margin-top: 30px; }
    a.button:hover { background-color: #0087A0; }
  </style>
  <script>
    function fetchTemp() {
      fetch('/readtemp')
        .then(response => response.text())
        .then(data => { document.getElementById('temp').innerHTML = data; });
    }
    setInterval(fetchTemp, 2000);
  </script>
</head>
<body onload='fetchTemp()'>
  <div class='container'>
    <h1>Temperature Monitor</h1>
    <div id='temp'>Loading...</div>
    <a class='button' href='/'>Move Motors</a>
  </div>
</body>
</html>
)rawliteral";

// Setup and initialize the system
void setup() {
  Serial.begin(9600);

  // BMP180 setup
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor!");
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Routes
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", html);
  });

  server.on("/move", HTTP_GET, []() {
    String motor1 = server.arg("motor1");
    String motor2 = server.arg("motor2");

    if (motor1.length() > 0) motor1Position = motor1.toInt();
    if (motor2.length() > 0) motor2Position = motor2.toInt();

    server.send(200, "text/html", "<div class='response'><h1>Motors Moved</h1><a href='/'>Go Back</a></div>");
  });

  server.on("/getMotorPositions", HTTP_GET, []() {
    String motorPositions = String(motor1Position) + "," + String(motor2Position);
    server.send(200, "text/plain", motorPositions);
  });

  server.on("/temp", HTTP_GET, []() {
    server.send_P(200, "text/html", tempHtml);
  });

  server.on("/readtemp", HTTP_GET, []() {
    float tempC = bmp.readTemperature();
    float tempF = tempC * 9.0 / 5.0 + 32.0;
    String data = "Celsius: " + String(tempC, 2) + " °C<br>Fahrenheit: " + String(tempF, 2) + " °F";
    server.send(200, "text/html", data);
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
