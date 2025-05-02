# Raspberry Pi Flask Web Controller

A web-based control interface for Raspberry Pi that includes:
- Stepper motor control
- Temperature monitoring (BMP180)
- Facial expression gallery/selector

Built using Python Flask for the backend and HTML/CSS/JavaScript for the frontend.

---

## Table of Contents

- [Project Structure](#project-structure)  
- [Installation](#installation)  
- [Running the App](#running-the-app)  
- [Code Walkthrough](#code-walkthrough)  
  - [`app.py`](#apppy---flask-backend)
  - [`index.html`](#indexhtml---motor-control-page)
  - [`temp.html`](#temphtml---temperature-monitoring-page)
  - [`faces.html`](#faceshtml---face-selection--gallery)
- [Customizing the Project](#customizing-the-project)
- [Static Files](#static-files)
- [Optional Features](#optional-features)
- [Endpoints Summary](#endpoints-summary)
- [Author and License](#author-and-license)

---

## Project Structure

/your-project-directory
│
├── app.py                 # Main Python Flask backend
├── static/                # Images, optional JS or CSS
│   └── images/
├── templates/
│   ├── index.html         # Motor control interface
│   ├── temp.html          # Temperature page
│   └── faces.html         # Face selection/gallery
└── README.md              # This file

---

## Installation

Install the required Python packages:

```bash
pip install flask smbus2 bme280  # or adafruit_bmp if using Adafruit's BMP library



⸻

Running the App

python3 app.py

Then visit:
http://<your-pi-ip>:5000/

⸻

Code Walkthrough

app.py – Flask Backend

Handles:
	•	Page routing
	•	I2C sensor reading (BMP180)
	•	Motor control commands

@app.route("/move")
def move_motor():
    motor = request.args.get("motor")
    direction = request.args.get("direction")
    print(f"Moving motor {motor} {direction}")
    return "OK"

Key Points:
	•	Uses smbus2 for I2C communication.
	•	Replace print() statements with GPIO control logic.
	•	Add routes to extend functionality (e.g., for face interaction or login).

⸻

index.html – Motor Control Page

A grid of circle buttons sends AJAX calls to the backend.

function moveMotor(motor, direction) {
  fetch(`/move?motor=${motor}&direction=${direction}`);
  localStorage.setItem(`motor${motor}`, direction);
}

How it Works:
	•	fetch() hits the /move route with motor number and direction.
	•	localStorage remembers last selection.

Customize:
	•	Add motors by duplicating .motor-control divs.
	•	Modify button layout/styles with CSS.
	•	Add animation or button feedback if needed.

⸻

temp.html – Temperature Monitoring Page

Reads temperature every 2 seconds using AJAX.

setInterval(fetchTemp, 2000);

function fetchTemp() {
  fetch('/readtemp')
    .then(response => response.text())
    .then(data => { document.getElementById('temp').innerHTML = data; });
}

Customize:
	•	Adjust refresh interval.
	•	Change color theme via CSS.
	•	Convert to Fahrenheit:

temp_f = temp_c * 9/5 + 32



⸻

faces.html – Face Selection / Gallery

Static gallery with image, title, and caption per face.

<div class="card" onclick="selectFace('happy')">
  <img src="face1.jpg">
  <h3>Happy</h3>
  <p>Smiling face</p>
</div>

Customize:
	•	Replace images in /static/images/
	•	Add more cards by duplicating the .card div.
	•	Add interactivity with onclick + a Flask route:

function selectFace(name) {
  fetch(`/face?name=${name}`);
}



⸻

Customizing the Project

Changing Images
	1.	Place your images in static/images/.
	2.	Update <img src="..."> tags in faces.html:

<img src="{{ url_for('static', filename='images/yourface.jpg') }}">



Changing Theme

Edit styles in each .html file under the <style> block. Modify:
	•	background-color
	•	Fonts
	•	Button shapes
	•	Container sizes

Adding Password Protection

Add this to app.py:

from flask import session, redirect

@app.before_request
def require_login():
    allowed_routes = ['login', 'static']
    if 'logged_in' not in session and request.endpoint not in allowed_routes:
        return redirect('/login')

Then create a /login route and login.html template that sets:

session['logged_in'] = True



⸻

Static Files

Place images in static/images/ and reference them like:

<img src="{{ url_for('static', filename='images/face1.jpg') }}">

Use static/ for:
	•	JS files
	•	External CSS
	•	Audio/video resources

⸻

Optional Features

Feature	Description
GPIO Control	Add actual hardware logic to /move
Face Selection Logic	Add route /face?name=xxx for handling
Fahrenheit Conversion	Convert sensor output from Celsius
Mobile Optimizations	All HTML uses responsive layout



⸻

Endpoints Summary

Endpoint	Method	Description
/	GET	Loads motor control page
/temp	GET	Loads temperature monitor
/faces	GET	Loads face gallery
/move	GET	Accepts motor + direction params
/readtemp	GET	Returns temperature as plain text
/face	GET	(Optional) Handle face selection