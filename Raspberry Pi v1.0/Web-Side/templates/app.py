from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

# Simulate motor positions (no GPIO interaction)
motor1_position = 0
motor2_position = 0

# Simulate temperature reading (no actual sensor)
temperature = 25.0  # Example temperature in Celsius

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/move', methods=['GET'])
def move_motors():
    global motor1_position, motor2_position
    motor1_position = int(request.args.get('motor1', 0))
    motor2_position = int(request.args.get('motor2', 0))
    
    # Return a response
    return jsonify({"message": "Motors moved", "motor1_position": motor1_position, "motor2_position": motor2_position})

@app.route('/getMotorPositions', methods=['GET'])
def get_motor_positions():
    # Return the current motor positions
    return jsonify({"motor1_position": motor1_position, "motor2_position": motor2_position})

@app.route('/temp', methods=['GET'])
def temp():
    return render_template('temp.html')

@app.route('/readtemp', methods=['GET'])
def read_temp():
    # Simulate reading temperature (no sensor)
    return jsonify({"temperature": temperature})

@app.route("/face")
def face_control():
    return render_template("face-control.html")


if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)