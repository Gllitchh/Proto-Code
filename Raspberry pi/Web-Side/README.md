This repository includes a Flask-based web server designed for Raspberry Pi, controlling stepper motors, reading temperature via a BMP180 sensor, and presenting a face-selection UI. The system uses HTML, CSS, and JavaScript on the frontend, with Python handling all backend logic and hardware communication.

This documentation is intended for developers or engineers who want to understand, maintain, or expand the codebase.

---

## Project Structure

/project-directory
│
├── app.py                 # Main Python Flask backend
├── static/
│   └── (images, JS, CSS if needed externally)
├── templates/
│   ├── index.html         # Motor control interface
│   ├── temp.html          # Temperature page
│   └── faces.html         # Face selection/gallery
└── README.md              # This file

---

## `app.py` – Flask Backend

### Purpose:
Handles routing, sensor reading, and (optionally) GPIO motor control.

### How it Works:
- Flask runs a local server on the Pi
- Routes serve each HTML page
- `/move` and `/readtemp` are AJAX endpoints hit by JavaScript

### Key Components:

```python
from flask import Flask, render_template, request
import smbus2
import bme280  # or use an Adafruit BMP180 library

	•	render_template() loads an HTML file from /templates
	•	request.args grabs parameters from the frontend
	•	smbus2 is used for I2C communication with the BMP180 sensor

Example Endpoint:

@app.route("/move")
def move_motor():
    motor = request.args.get("motor")
    direction = request.args.get("direction")
    print(f"Moving motor {motor} {direction}")
    return "OK"

To activate actual GPIO:
Import RPi.GPIO or gpiozero and map the motor index to GPIO pins.

⸻

index.html – Motor Control Page

Purpose:

Provides circular buttons to control motors and their directions.

How it Works:
	•	Each motor (1–6) has 4 direction buttons: up, down, left, right
	•	On click, it sends a GET request to /move with the selected motor and direction

Key Elements:

<button onclick="moveMotor(1, 'up')">↑</button>

function moveMotor(motor, direction) {
  fetch(`/move?motor=${motor}&direction=${direction}`);
  localStorage.setItem(`motor${motor}`, direction);
}

	•	localStorage is used to remember the last direction used, even after page reloads.

To Customize:
	•	Add more motors: Duplicate a .motor-control block and update motor number.
	•	Change layout/colors: Edit the .circle, .motor-control, and body styles.
	•	Secure the page: Add password protection by checking credentials on the Flask route or add login with Flask sessions.

⸻

temp.html – Temperature Monitoring Page

Purpose:

Displays the current temperature from the BMP180 sensor, refreshed every 2 seconds.

How it Works:
	•	On page load, JavaScript calls /readtemp
	•	The response is inserted into the #temp element

Key JavaScript:

function fetchTemp() {
  fetch('/readtemp')
    .then(response => response.text())
    .then(data => { document.getElementById('temp').innerHTML = data; });
}
setInterval(fetchTemp, 2000);

	•	You can change the refresh rate by modifying 2000 (in milliseconds).

To Customize:
	•	Style/Theme: Change container background, font sizes in CSS.
	•	Units: Convert Celsius to Fahrenheit if needed:

temp_f = temp_c * 9/5 + 32



⸻

faces.html – Face Selection / Gallery

Purpose:

Displays a set of facial expression cards (images and descriptions).

How it Works:
	•	Each face is a styled .card with an image, heading, and caption.
	•	You can add onclick events to send signals or record which face was selected.

Card Structure:

<div class="card">
  <img src="face1.jpg" alt="Face 1">
  <h3>Face 1</h3>
  <p>This face reflects joy.</p>
</div>

To Customize:
	•	Change Images:
	•	Replace face1.jpg, face2.jpg, etc. in the /static/ folder
	•	Update the <img src="..."> paths accordingly
	•	Add More Faces:
	•	Copy-paste one <div class="card"> block
	•	Update image file, heading, and description
	•	Make Faces Interactive:

<div class="card" onclick="selectFace('happy')">

function selectFace(name) {
  fetch(`/face?name=${name}`);
}


	•	Trigger GPIO on Face Selection: Add a Flask route /face?name=happy and trigger hardware logic accordingly

⸻

Sidebar Navigation (Used in All Pages)

How it Works:
	•	Toggles with a hamburger button (☰)
	•	Slides from the left with CSS transitions
	•	Same links on all pages to provide navigation

To Customize:
	•	Add new menu items by inserting links in all 3 HTML pages:

<a href="/yournewpage">Your Page</a>

	•	Modify styling in the .sidebar and .openbtn CSS blocks.

⸻

Static Files (Images, JS, CSS)

If you want to move all assets to a separate static/ folder, update paths in your HTML like this:

<img src="{{ url_for('static', filename='images/face1.jpg') }}">

Then store images in static/images/face1.jpg.

⸻

Password Protection (Optional)

If you want to protect your control interface:

Simple (not secure, just for basic use):

In app.py:

@app.before_request
def require_login():
    allowed_routes = ['login', 'static']
    if 'logged_in' not in session and request.endpoint not in allowed_routes:
        return redirect('/login')

Add a /login route and login.html page that sets session['logged_in'] = True.

⸻

How to Run

pip install flask smbus2 bme280  # or your BMP library
python3 app.py

Open in browser:

http://<raspberrypi-ip>:5000/



⸻

Summary of Variables and Endpoints

Variable / ID	Description
motor	Which motor is being moved (1–6)
direction	Direction command (up, down, left, right)
#temp	Div where temperature is inserted
localStorage	Stores last motor direction per motor

Endpoint	Purpose
/	Loads motor control page
/temp	Loads temperature monitor page
/faces	Shows face selection gallery
/readtemp	Returns sensor data in text format
/move	Accepts motor number and direction as params