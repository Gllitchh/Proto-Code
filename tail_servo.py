# Import libraries
import RPi.GPIO as GPIO
import time
# Set GPIO numbering mode
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)

tail_part = 0
angle = True
angle2 = True
auto_move = False
place1 = 0
place2 = 0

def first_tail_part_servos():
    GPIO.setup(11,GPIO.OUT)# Set pin 11 as an output, and define as servo1 as PWM p
    servo1 = GPIO.PWM(11,50)# pin 11 for servo1, pulse 50Hz
    GPIO.setup(12,GPIO.OUT)
    servo2 = GPIO.PWM(12,50)
    servo1.start(0)
    servo2.start(0)
    if place_first == 1:
        servo1.ChangeDutyCycle(7+(first_right_angle/-18))# To make the servos work togather put a - in front of the 18, thats servo you want to change.
        servo2.ChangeDutyCycle(7+(first_right_angle/-18))# Or put a - in front of both to make them go backwards
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(first_left_angle/18))
        servo2.ChangeDutyCycle(7+(first_left_angle/18))
    elif place_first == 0:
        servo1.ChangeDutyCycle(7+(first_left_angle/18))
        servo2.ChangeDutyCycle(7+(first_left_angle/18))
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(first_right_angle/-18))
        servo2.ChangeDutyCycle(7+(first_right_angle/-18))
    time.sleep(0.3)
    servo2.ChangeDutyCycle(7)
    servo1.ChangeDutyCycle(7)
    time.sleep(0.1)
    servo1.stop()
    servo2.stop()

def seccond_tail_part_servos():
    GPIO.setup(11,GPIO.OUT)
    servo1 = GPIO.PWM(11,50)
    GPIO.setup(12,GPIO.OUT)
    servo2 = GPIO.PWM(12,50)
    servo1.start(0)# Start PWM running, with value of 0 (pulse off)
    servo2.start(0)
    if place_seccond == 1:
        servo1.ChangeDutyCycle(7+(seccond_right_angle/-18))
        servo2.ChangeDutyCycle(7+(seccond_right_angle/-18))
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(seccond_left_angle/18))
        servo2.ChangeDutyCycle(7+(seccond_left_angle/18))
    elif place_seccond ==0:
        servo1.ChangeDutyCycle(7+(seccond_left_angle/18))
        servo2.ChangeDutyCycle(7+(seccond_left_angle/18))
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(seccond_right_angle/-18))
        servo2.ChangeDutyCycle(7+(seccond_right_angle/-18))
    time.sleep(0.3)
    servo2.ChangeDutyCycle(7)# puts the servos back to point 0
    servo1.ChangeDutyCycle(7)
    time.sleep(0.1)
    servo1.stop()# stops the servo so it doesn't brake its self
    servo2.stop()
        
def third_tail_part_servos():#these are to conterol and setup the servos.
    GPIO.setup(11,GPIO.OUT)
    servo1 = GPIO.PWM(11,50)
    GPIO.setup(12,GPIO.OUT)
    servo2 = GPIO.PWM(12,50)
    servo1.start(0)
    servo2.start(0)
    if place_third == 1:
        servo1.ChangeDutyCycle(7+(third_right_angle/-18))
        servo2.ChangeDutyCycle(7+(third_right_angle/-18))
        time.sleep(0.3)
        servo1.ChangeDutyCycle(7+(third_left_angle/18))
        servo2.ChangeDutyCycle(7+(third_left_angle/18))
    elif place_third ==0:
        servo1.ChangeDutyCycle(7+(third_left_angle/18))
        servo2.ChangeDutyCycle(7+(third_left_angle/18))
        time.sleep(0.3)# slows down the software. So it doesn't kill its self
        servo1.ChangeDutyCycle(7+(third_right_angle/-18))
        servo2.ChangeDutyCycle(7+(third_right_angle/-18))
    time.sleep(0.3)
    servo2.ChangeDutyCycle(7)
    servo1.ChangeDutyCycle(7)
    time.sleep(0.1)
    servo1.stop()
    servo2.stop()
    
while angle:# puts all the code into a loop that won't stop untilll its set to False
    while angle2:# I couldn't think of a differnt way to do it :/
        place = input('which way do you want the tail to move from. Please use 1 as Right to Left and 2 as Left and Right:')
        if ('1') in place:# takes user input and puts it into variables 
            place1 = 1
            angle2 = False
        elif ('2') in place:
            place2 = 2
            angle2 = False
        else:
            print('sorry thats not right') # tells you that its not right :|
        
    right_angle = float(input('how much do you want the tail to move to the right between 0 and 90:'))# It's an input for you data
        
    left_angle = float(input('how much do you want the tail to move to the left between 0 and 90:'))
    
    # Lets the program know were it is in the setup.
    if tail_part ==0:
        first_right_angle = right_angle
        first_left_angle = left_angle 
        place_first = place1
        place1 = 0
        angle2 = True
        print('ok now doing a preview')
        first_tail_part_servos()
    elif tail_part ==1:
        seccond_right_angle = right_angle
        seccond_left_angle = left_angle
        place_seccond = place1
        place1 = 0
        angle2 = True
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
    elif tail_part >=2:
        angle = False
        angle2 = False
    change = input('whould you like to change the angle or tail movemet?:')
    if ('y') in change:
        angle = True
        print('ok')
        place1 = 0
        place2 = 0
    else:
        tail_part = tail_part + 1
        
        
auto = input('are you ready to put it into auto mode?:')
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
    
try:
    while auto_move:
        first_tail_part_servos()# calls them
        seccond_tail_part_servos()
        third_tail_part_servos()

except:#Clean things up at the end
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
clean()# calls clean