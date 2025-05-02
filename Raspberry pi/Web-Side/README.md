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
  - [app.py - Flask Backend](#apppy---flask-backend)
  - [index.html - Motor Control Page](#indexhtml---motor-control-page)
  - [temp.html - Temperature Monitoring Page](#temphtml---temperature-monitoring-page)
  - [faces.html - Face Selection / Gallery](#faceshtml---face-selection--gallery)
- [Customizing the Project](#customizing-the-project)
- [Static Files](#static-files)
- [Optional Features](#optional-features)
- [Endpoints Summary](#endpoints-summary)
- [Author and License](#author-and-license)

---

## Project Structure

The project directory is structured as follows:

/your-project-directory
│
├── app.py                 # Main Python Flask backend
├── static/                # Images, optional JS or CSS
│   └── images/            # Directory for storing images
├── templates/             # HTML templates
│   ├── index.html         # Motor control interface
│   ├── temp.html          # Temperature page
│   └── faces.html         # Face selection/gallery
└── README.md              # This file

---

## Installation

To run this project, you need to install the necessary dependencies. Install them using pip:

```bash
pip install flask smbus2 bme280  # or adafruit_bmp if using Adafruit's BMP library



⸻

Running the App

To run the application, simply execute:

python3 app.py

Once the server is running, visit the following URL in your browser:

http://<your-pi-ip>:5000/



⸻

Code Walkthrough

app.py – Flask Backend

This file handles the backend functionality of the app, including routes for motor control, temperature reading, and facial expression selection. It uses Flask to route requests and smbus2 to communicate with the BMP180 temperature sensor.

Key Routes:
	•	/move: Handles motor movement requests by accepting motor number and direction parameters.
	•	/readtemp: Reads temperature data from the BMP180 sensor and returns it as plain text.
	•	/face: (Optional) Handles face selection requests.

Example for controlling motors:

@app.route("/move")
def move_motor():
    motor = request.args.get("motor")
    direction = request.args.get("direction")
    print(f"Moving motor {motor} {direction}")
    return "OK"



⸻

index.html – Motor Control Page

This is the main page for controlling motors. It displays a grid of circle buttons. Each button, when clicked, sends an AJAX request to the /move route to control a specific motor’s direction.

Key Features:
	•	Circle buttons that represent each motor.
	•	Local storage is used to remember the last selected motor direction.
	•	Responsive layout adapts to different screen sizes.

Example button:

<div class="circle-button" onclick="moveMotor(1, 'forward')">Motor 1</div>

Customization:
	•	Add more motors by copying the button divs and adjusting the moveMotor() function call.
	•	Modify the button layout/styles via the embedded CSS.

⸻

temp.html – Temperature Monitoring Page

This page displays the current temperature, updated every 2 seconds, using AJAX to call the /readtemp route.

Key Features:
	•	Fetches temperature every 2 seconds using JavaScript.
	•	Displays the temperature in a readable format.

Example code for fetching and displaying the temperature:

setInterval(fetchTemp, 2000);

function fetchTemp() {
  fetch('/readtemp')
    .then(response => response.text())
    .then(data => { document.getElementById('temp').innerHTML = data; });
}

Customization:
	•	Change the temperature refresh rate by modifying the setInterval() time.
	•	Modify the design and appearance through CSS.

⸻

faces.html – Face Selection / Gallery

This page displays a gallery of faces (images) with a title and description. When a face is clicked, a request is sent to the backend to register the face selection.

Key Features:
	•	Clickable cards that each represent a different face.
	•	The face images are displayed using a grid layout.

Example card:

<div class="card" onclick="selectFace('happy')">
  <img src="face1.jpg">
  <h3>Happy</h3>
  <p>Smiling face</p>
</div>

Customization:
	•	Replace the images with your own by adding them to the static/images/ directory.
	•	Update the text in each card (title, description) to suit your preferences.

⸻

Customizing the Project

Changing Images
	1.	Place your custom images in the static/images/ directory.
	2.	Update the src attribute of <img> tags in faces.html:

<img src="{{ url_for('static', filename='images/yourimage.jpg') }}">



Changing Theme

Modify the styles in each HTML file under the <style> block:
	•	Change background-color, font-family, button styles, etc.
	•	You can also modify the grid layouts and spacing for a custom look.

Adding Password Protection

To add basic authentication to your app, modify the app.py file:

from flask import session, redirect

@app.before_request
def require_login():
    allowed_routes = ['login', 'static']
    if 'logged_in' not in session and request.endpoint not in allowed_routes:
        return redirect('/login')

Then create a /login route and a corresponding login.html template that sets:

session['logged_in'] = True



⸻

Static Files

Place your static files such as images, JavaScript, and custom CSS in the static/ directory. The images should be placed under static/images/ for easy reference.

Example of referencing a static image in faces.html:

<img src="{{ url_for('static', filename='images/face1.jpg') }}">



⸻

Optional Features

Feature	Description
GPIO Control	Implement actual hardware control for motors.
Face Selection Logic	Handle face selection dynamically via routes.
Temperature in Fahrenheit	Convert the sensor output to Fahrenheit.
Mobile Optimization	Ensure pages are fully responsive on mobile.



⸻

Endpoints Summary

Endpoint	Method	Description
/	GET	Loads motor control page
/temp	GET	Loads the temperature monitoring page
/faces	GET	Loads the face gallery page
/move	GET	Controls the motor direction
/readtemp	GET	Fetches temperature data from BMP180 sensor
/face	GET	(Optional) Handles face selection