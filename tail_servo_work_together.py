# This program runs the servos together.



# Import libraries
import RPi.GPIO as GPIO
import time
import sys
# Set GPIO trig and echo for a range finder.

trig =()# please put in the port that the range finder trig is in.
echo =()# please put in the port that the range finder echo is in.

# Set GPIO numbering mode
GPIO.setmode(GPIO.BOARD)
GPIO.setup(trig,GPIO.OUT)# this is for the trig. data leaves from this 'port'
GPIO.setup(echo,GPIO.IN)# this is for the echo. data enters here 
GPIO.setwarnings(False)
# Variables
tail_part = 0
place1 = 0
place2 = 0


servo_1_port = (11)# please put in the numbers of the ports that the servos are in. In each one.
servo_2_port = (12)#servo 1 and 2 are the first ones that will be moved. Servo3 and 4 are seccond, and servo 5 and 6 are last to move. 
servo_3_port = (13)#keep this in mind when puting in your ports.
servo_4_port = (15)
servo_5_port = (16)
servo_6_port = (7)

angle = True
angle2 = True
auto_move = False
servo_port = True


def first_tail_part_servos():
    GPIO.setup(servo_1_port,GPIO.OUT)# Set pin 11 as an output, and define as servo1 as PWM p
    servo1 = GPIO.PWM(servo_1_port,50)# pin 11 for servo1, pulse 50Hz
    GPIO.setup(servo_2_port,GPIO.OUT)
    servo2 = GPIO.PWM(servo_2_port,50)
    servo1.start(0)
    servo2.start(0)
    if place_first == 1:
        servo1.ChangeDutyCycle(7+(first_right_angle/-18))# To make the servos work togather put a - in front of the 18, thats servo you want to change.
        servo2.ChangeDutyCycle(7+(first_right_angle/18))# Or put a - in front of both to make them go backwards
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(first_left_angle/18))
        servo2.ChangeDutyCycle(7+(first_left_angle/-18))
    elif place_first == 0:
        servo1.ChangeDutyCycle(7+(first_left_angle/-18))
        servo2.ChangeDutyCycle(7+(first_left_angle/18))
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(first_right_angle/18))
        servo2.ChangeDutyCycle(7+(first_right_angle/-18))
    time.sleep(0.3)
    servo2.ChangeDutyCycle(0)
    servo1.ChangeDutyCycle(0)
    time.sleep(0.1)
    servo1.stop()
    servo2.stop()

def seccond_tail_part_servos():
    GPIO.setup(servo_3_port,GPIO.OUT)
    servo1 = GPIO.PWM(servo_3_port,50)
    GPIO.setup(servo_4_port,GPIO.OUT)
    servo2 = GPIO.PWM(servo_4_port,50)
    servo1.start(0)# Start PWM running, with value of 0 (pulse off)
    servo2.start(0)
    if place_seccond == 1:
        servo1.ChangeDutyCycle(7+(seccond_right_angle/-18))
        servo2.ChangeDutyCycle(7+(seccond_right_angle/18))
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(seccond_left_angle/18))
        servo2.ChangeDutyCycle(7+(seccond_left_angle/-18))
    elif place_seccond ==0:
        servo1.ChangeDutyCycle(7+(seccond_left_angle/-18))
        servo2.ChangeDutyCycle(7+(seccond_left_angle/18))
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(seccond_right_angle/18))
        servo2.ChangeDutyCycle(7+(seccond_right_angle/-18))
    time.sleep(0.3)
    servo2.ChangeDutyCycle(0)# puts the servos back to point 0
    servo1.ChangeDutyCycle(0)
    time.sleep(0.1)
    servo1.stop()# stops the servo so it doesn't brake its self
    servo2.stop()
        
def third_tail_part_servos():#these are to conterol and setup the servos.
    GPIO.setup(servo_5_port,GPIO.OUT)
    servo1 = GPIO.PWM(servo_5_port,50)
    GPIO.setup(servo_6_port,GPIO.OUT)
    servo2 = GPIO.PWM(servo_6_port,50)
    servo1.start(0)
    servo2.start(0)
    if place_third == 1:
        servo1.ChangeDutyCycle(7+(third_right_angle/-18))
        servo2.ChangeDutyCycle(7+(third_right_angle/18))
        time.sleep(0.3)# you can change these to your liking.
        servo1.ChangeDutyCycle(7+(third_left_angle/18))
        servo2.ChangeDutyCycle(7+(third_left_angle/-18))
    elif place_third ==0:
        servo1.ChangeDutyCycle(7+(third_left_angle/-18))
        servo2.ChangeDutyCycle(7+(third_left_angle/18))
        time.sleep(0.3)# slows down the software. So it doesn't kill its self
        servo1.ChangeDutyCycle(7+(third_right_angle/18))
        servo2.ChangeDutyCycle(7+(third_right_angle/-18))
    time.sleep(0.3)
    servo2.ChangeDutyCycle(0)
    servo1.ChangeDutyCycle(0)
    time.sleep(0.1)
    servo1.stop()
    servo2.stop()

def servo_input():
    global servo_1_port, servo_2_port, servo_3_port, servo_4_port, servo_5_port, servo_6_port
    servo_port_anser = input('did you put in the ports for the servos? ')
    if ('y') in servo_port_anser:
        servo_port_anser =  input('did you put in the ranger finder ports?: ')
        if ('y') in servo_port_anser:
            print('ok')
        else:
            print('')
            trig = int(input('ok plaese enter the trig port on the range finder: '))
            echo = int(input('please enter the echo port on it to: '))
    else:
        servo_port_anser = input('do you want to put them in here or no? ')
        if ('h') in servo_port_anser:
            print('')
            print('''servo 1 and 2 are the first ones that will be moved.
Servo3 and 4 are seccond, and servo 5 and 6 are last to move.
Please keep this in mind.''')
            print('')
            servo_1_port = int(input('ok this is for servo1: '))
            servo_2_port = int(input('now this is for servo2: '))
            servo_3_port = int(input('this is for servo3: '))
            servo_4_port = int(input('please be care full. This is for servo4: '))
            servo_5_port = int(input('this one is for servo5: '))
            servo_6_port = int(input('now this is for servo6: '))
            servo_port_anser = input('did you put in the ranger finder ports?: ')
            if ('y') in servo_port_anser:
                print('ok')
            else:
                print('')
                trig = int(input('ok plaese enter the trig port on the range finder: '))
                echo = int(input('please enter the echo port on it to: '))
            
            
    
servo_input()

print('there is 3 of these questions you need to anser')

while angle:# puts all the code into a loop that won't stop untill its set to False
        
    while angle2:# I couldn't think of a differnt way to do it :/
        place = input('which way do you want the tail to move from. Please use 1 as Right to Left and 2 as Left and Right: ')
        if ('1') in place:# takes user input and puts it into variables 
            place1 = 1
            angle2 = False
        elif ('2') in place:
            place2 = 2
            angle2 = False
        else:
            print('sorry thats not right') # tells you that its not right :|
   
        
    right_angle = float(input('how much do you want the tail to move to the right between 0 and 90: '))# It's an input for you data
        
    left_angle = float(input('how much do you want the tail to move to the left between 0 and 90: '))
    
    # Lets the program know were it is in the setup.
    if tail_part ==0:
        first_right_angle = right_angle
        first_left_angle = left_angle 
        place_first = place1
        place1 = 0
        angle2 = True
        servo_port = True
        print('ok now doing a preview')
        first_tail_part_servos()
    elif tail_part ==1:
        seccond_right_angle = right_angle
        seccond_left_angle = left_angle
        place_seccond = place1
        place1 = 0
        angle2 = True
        servo_port = True
        print('ok now doing a preview')
        seccond_tail_part_servos()
    elif tail_part ==2:
        third_right_angle = right_angle
        third_left_angle = left_angle
        place_third = place1
        place1 = 0
        print('ok now doing a preview')
        third_tail_part_servos()
        angle2 = True
        angle = False
        servo_port = False
    elif tail_part >=2:
        angle = False
        angle2 = False
        servo_port = False
    change = input('would you like to change the angle or tail movemet?: ')
    if ('y') in change:
        angle = True
        servo_port = True
        print('ok')
        place1 = 0
        place2 = 0
    else:
        tail_part = tail_part + 1
        
        
auto = input('are you ready to put it into auto mode?: ')
if ('y') in auto:
    print('ok here we go')
    print('runing tests...')
    first_tail_part_servos()
    time.sleep(0.5)
    seccond_tail_part_servos()
    time.sleep(0.5)
    third_tail_part_servos()
    print('all systems good.')
    time.sleep(1)
    auto_move = True
else:
    print('ok if thats what you want')
    print('restarting...')
    

while auto_move:
        
    GPIO.output(trig, False)# set this to the number you set for trig in the begining
    print("Waiting For Sensor To Settle")
    time.sleep(2)

    GPIO.output(trig, True)
    time.sleep(0.00001)# you can change these, but you may brake it.
    GPIO.output(trig, False)

    while GPIO.input(echo)==0:# set this to the number you set for echo in the begining
        pulse_start = time.time()

    while GPIO.input(echo)==1:
        pulse_end = time.time()

    pulse_duration = pulse_end - pulse_start

    distance = pulse_duration * 17150

    distance = round(distance, 2)

    print("Distance:",distance,"cm")
        
        
    if distance <=3.5:
        first_tail_part_servos()
        seccond_tail_part_servos()
        third_tail_part_servos()
            
    else:
        distance = 0

clean = True
            
while clean:#Clean things up at the end
    GPIO.cleanup()
    print("Goodbye!")
    auto_move = False

def clean():# cleans the pins on the pi
    GPIO.setmode(GPIO.BOARD)    
    GPIO.setup(11,GPIO.OUT)
    servo1 = GPIO.PWM(11,50)
    GPIO.setup(12,GPIO.OUT)
    servo2 = GPIO.PWM(12,50)
    servo1.start(0)
    servo2.start(0)
    servo1.ChangeDutyCycle(7+(0/18))
    servo2.ChangeDutyCycle(7+(0/18))
    time.sleep(0.4)
    servo2.ChangeDutyCycle(7)
    servo1.ChangeDutyCycle(7)
    time.sleep(0.4)
    GPIO.cleanup()
    clean = False
clean()# calls clean