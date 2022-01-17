import bmpsensor
import time
while True:
    temp, pressure = bmpsensor.readBmp180()
    
    print("Pressure is ",pressure) # Pressure in Pa 
    print("\n")
    time.sleep(1)