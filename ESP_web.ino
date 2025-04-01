#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Wi-Fi credentials
const char* ssid = "TCC Willis";
const char* password = "7157160546";

// Create an instance of the web server
ESP8266WebServer server(80);

// Web page HTML code with Portainer-like theme and AJAX functionality
const char* html = "<html>\
<head>\
  <title>Motor Control</title>\
  <style>\
    body { background-color: #1D1F27; color: #FFFFFF; font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; }\
    h1 { color: #00A9E0; margin-top: 50px; }\
    .container { width: 50%; margin: 0 auto; padding: 20px; background: #2A2E3B; border-radius: 10px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2); }\
    label { font-size: 18px; display: block; margin-top: 10px; text-align: left; color: #A0A0A0; }\
    input[type='number'] { width: 90%; padding: 12px; margin-top: 5px; border: none; border-radius: 5px; font-size: 18px; background-color: #3A3D47; color: #FFFFFF; border: 1px solid #444; }\
    input[type='number']:focus { background-color: #505460; border-color: #00A9E0; outline: none; }\
    input[type='submit'] { background-color: #00A9E0; color: #FFFFFF; padding: 12px 24px; border: none; border-radius: 5px; font-size: 18px; cursor: pointer; }\
    input[type='submit']:hover { background-color: #0087A0; }\
    .response { background-color: #2A2E3B; padding: 20px; border-radius: 10px; margin-top: 30px; box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2); }\
    a { color: #00A9E0; text-decoration: none; font-size: 18px; display: inline-block; margin-top: 20px; }\
    a:hover { color: #0087A0; }\
    .circle-container { display: flex; justify-content: space-around; }\
    .circle { margin: 20px; position: relative; width: 150px; height: 150px; background-color: #3A3D47; border-radius: 50%; border: 10px solid #444; }\
    .circle > canvas { position: absolute; top: 0; left: 0; width: 100%; height: 100%; }\
    .label { font-size: 16px; color: #A0A0A0; margin-top: 10px; }\
  </style>\
  <script>\
    var motor1Angle = 0;\
    var motor2Angle = 0;\
    function moveMotors() {\
      var motor1 = document.getElementById('motor1').value;\
      var motor2 = document.getElementById('motor2').value;\
      var xhr = new XMLHttpRequest();\
      xhr.open('GET', '/move?motor1=' + motor1 + '&motor2=' + motor2, true);\
      xhr.onreadystatechange = function() {\
        if (xhr.readyState == 4 && xhr.status == 200) {\
          document.getElementById('response').innerHTML = xhr.responseText;\
          updateCircles(motor1, motor2);\
        }\
      };\
      xhr.send();\
      return false;\
    }\
    function updateCircles(motor1, motor2) {\
      motor1Angle = parseInt(motor1);\
      motor2Angle = parseInt(motor2);\
      drawCircle('motor1Canvas', motor1Angle);\
      drawCircle('motor2Canvas', motor2Angle);\
    }\
    function drawCircle(canvasId, angle) {\
      var canvas = document.getElementById(canvasId);\
      var ctx = canvas.getContext('2d');\
      var radius = canvas.width / 2;\
      var startAngle = -0.5 * Math.PI;\
      var endAngle = (angle / 360) * 2 * Math.PI + startAngle;\
      if (angle < 0) {\
        endAngle = startAngle + (angle / 360) * 2 * Math.PI;\
      }\
      ctx.clearRect(0, 0, canvas.width, canvas.height);\
      ctx.beginPath();\
      ctx.arc(radius, radius, radius - 10, startAngle, endAngle, angle < 0);\
      ctx.lineWidth = 10;\
      ctx.strokeStyle = '#00A9E0';\
      ctx.stroke();\
    }\
  </script>\
</head>\
<body>\
  <div class='container'>\
    <h1>Stepper Motor Control</h1>\
    <form onsubmit='return moveMotors()'>\
      <label for='motor1'>Motor 1 Angle (-360 to 360): </label>\
      <input type='number' id='motor1' name='motor1' min='-360' max='360'><br><br>\
      <label for='motor2'>Motor 2 Angle (-360 to 360): </label>\
      <input type='number' id='motor2' name='motor2' min='-360' max='360'><br><br>\
      <input type='submit' value='Move Motors'>\
    </form>\
    <div class='circle-container'>\
      <div>\
        <div class='label'>Motor 1</div>\
        <div class='circle'>\
          <canvas id='motor1Canvas' width='150' height='150'></canvas>\
        </div>\
      </div>\
      <div>\
        <div class='label'>Motor 2</div>\
        <div class='circle'>\
          <canvas id='motor2Canvas' width='150' height='150'></canvas>\
        </div>\
      </div>\
    </div>\
    <div id='response'></div>\
  </div>\
</body>\
</html>";

void setup() {
  // Start Serial communication with Uno (TX/RX)
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Handle requests
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", html);
  });

  server.on("/move", HTTP_GET, []() {
    // Read motor angles from the URL
    String motor1 = server.arg("motor1");
    String motor2 = server.arg("motor2");
    String command = "";

    // Check if the motor angles are not empty before sending
    if (motor1.length() > 0) {
      command += "motor1=" + motor1 + ";";
    }
    if (motor2.length() > 0) {
      command += "motor2=" + motor2 + ";";
    }

    // Send only if there is a valid command
    if (command.length() > 0) {
      Serial.println(command);
    }

    // Send a response back to the same page
    server.send(200, "text/html", "<div class='response'><h1>Motors Moved</h1><a href='/'>Go Back</a></div>");
  });

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
