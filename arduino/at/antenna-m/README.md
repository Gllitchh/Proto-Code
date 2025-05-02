## 📦 Features

- 🌐 Web-based UI to control stepper motors
- 💾 Save motor angles to EEPROM
- 🌡️ Live temperature monitoring (BMP180)
- 🔐 Password-protected EEPROM reset
- 🎨 Dynamic visual angle display using Canvas

---

## 🔧 Libraries Used

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <EEPROM.h>
```

Install these libraries via the **Arduino Library Manager**.

---

## 🔌 Setup Instructions

### 1. Wi-Fi Credentials
Replace these lines in the code:
```cpp
const char* ssid = "wifi name here";
const char* password = "wifi password here";
```

### 2. Upload the Code
Upload the code to your ESP8266 using the Arduino IDE.

### 3. Open Serial Monitor
Watch for:
```
Connected to WiFi
192.168.x.x
Server started
```

Navigate to the displayed IP in your browser.

---

## 🌐 Web Interface

### `/`
**Main control panel:**
- Enter angles (-360° to 360°) for Motor 1 and Motor 2
- Visualize positions using circular indicators
- Reset EEPROM via password prompt
- Link to temperature page

### `/temp`
**Live temperature monitor:**
- Updates every 2 seconds
- Shows Celsius and Fahrenheit

---

## 📂 EEPROM Use

| Address | Purpose          |
|---------|------------------|
| `0`     | Motor 1 position |
| `4`     | Motor 2 position |

EEPROM stores last motor angles across power cycles.

---

## 🔐 EEPROM Reset

- Triggered from UI button
- Protected by JS prompt:
```js
if (password === "mypassword")
```
You can change this password in the code:
```cpp
const String correctPassword = "mypassword";
```

---

## 📡 API Routes

| Route              | Method | Description                     |
|-------------------|--------|---------------------------------|
| `/`               | GET    | Motor control page              |
| `/move`           | GET    | Move motors, update EEPROM      |
| `/getMotorPositions` | GET | Get saved motor positions       |
| `/clearEEPROM`    | GET    | Reset motor positions to 0      |
| `/temp`           | GET    | Temperature UI page             |
| `/readtemp`       | GET    | Returns temp in C and F         |

---

## 🛠️ Serial Output

The ESP8266 sends formatted data to serial:
motor1=90;motor2=180;