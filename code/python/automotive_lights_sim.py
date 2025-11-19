from vpython import *
import tkinter as tk
import numpy as np
import serial
import time
from enum import Enum

class State(Enum):
    OFF = "OFF"
    AUTO = "AUTO"
    ON = "ON"

class Fault(Enum):
    CRITICAL = "CRITICAL_FAULT"
    DASH = "DASH_FAULT"
    BACK = "BACK_FAULT"

class AutoMode(Enum):
    ON = "AUTO_ON"
    OFF = "AUTO_OFF"

scene.camera.pos = vector(3, 1.5, 3)     # desno, iznad i malo ispred
scene.camera.axis = vector(-3, -1.5, -3) # gleda prema centru auta

carBottomX = 1.8
carBottomY = 0.7
carBottomZ = 4

carTopX = carBottomX
carTopY = carBottomY*0.85
carTopZ = carBottomZ*0.6

tireLen = carBottomX/8
tireRadius = carBottomY/2.5

headLightX = carBottomX/6
headLightY = headLightX/3
headLightZ = headLightX/90
brakeLightX = headLightX*4.5
brakeLightY = headLightY/2
brakeLightRadius = brakeLightY*2
brakeLightLen = headLightZ*2

offLightsColor = vector(0.15, 0.15, 0.15)
onLightsColor = vector(0.95, 0.95, 1)
carColor = vector(0.4, 0.4, 0.4)

headLightLeft = box(size = vector(headLightX, headLightY, headLightZ), color = offLightsColor, pos = vector(-carBottomX/2*0.75, carBottomY/2*0.5, carBottomZ/2))
headLightCenter = box(size = vector(carBottomX*0.75, headLightY/4, headLightZ), color = offLightsColor, pos = vector(0, carBottomY/2*0.5+headLightY/2-headLightY/8, carBottomZ/2))
headLightRight = box(size = vector(headLightX, headLightY, headLightZ), color = offLightsColor, pos = vector(carBottomX/2*0.75, carBottomY/2*0.5, carBottomZ/2))
brakeLightCenter = box(size = vector(brakeLightX, brakeLightY, headLightZ), color = color.red, pos = vector(0, carBottomY/2*0.7, -carBottomZ/2))
brakeLightLeft = box(size = vector(brakeLightY, brakeLightY*3.5, headLightZ), color = color.red, pos = vector(brakeLightX/2-brakeLightY/2, carBottomY/2*0.7-brakeLightY*3.5/2, -carBottomZ/2))
brakeLightRight = box(size = vector(brakeLightY, brakeLightY*3.5, headLightZ), color = color.red, pos = vector(-brakeLightX/2+brakeLightY/2, carBottomY/2*0.7-brakeLightY*3.5/2, -carBottomZ/2))
brakeLightLeftCylGlass = cylinder(radius = brakeLightRadius, length = brakeLightLen, color = vector(0.15, 0.15, 0.15), pos = vector(brakeLightX/2-brakeLightY/2, carBottomY/2*0.7-brakeLightY*3.5, -carBottomZ/2), axis = vector(0, 0, -1))
brakeLightRightCylGlass = cylinder(radius = brakeLightRadius, length = brakeLightLen, color = vector(0.15, 0.15, 0.15), pos = vector(-brakeLightX/2+brakeLightY/2, carBottomY/2*0.7-brakeLightY*3.5, -carBottomZ/2), axis = vector(0, 0, -1))
brakeLightLeftCyl = cylinder(radius = brakeLightRadius*0.65, length = brakeLightLen*2, color = color.red, pos = vector(brakeLightX/2-brakeLightY/2, carBottomY/2*0.7-brakeLightY*3.5, -carBottomZ/2), axis = vector(0, 0, -1))
brakeLightRightCyl = cylinder(radius = brakeLightRadius*0.65, length = brakeLightLen*2, color = color.red, pos = vector(-brakeLightX/2+brakeLightY/2, carBottomY/2*0.7-brakeLightY*3.5, -carBottomZ/2), axis = vector(0, 0, -1))

carBottom = box(size = vector(carBottomX, carBottomY, carBottomZ), color = carColor)
carTop = box(size = vector(carTopX, carTopY, carTopZ), color = carColor, pos = vector(0, carTopY, 0))
tireFrontLeft = cylinder(radius = tireRadius, length = tireLen, color = color.black, pos = vector(carBottomX/2-tireLen*0.9, -carBottomY/2+tireRadius/2.5, carBottomZ/3.5))
tireFrontRight = cylinder(radius = tireRadius, length = tireLen, color = color.black, pos = vector(-carBottomX/2-tireLen*0.1, -carBottomY/2+tireRadius/2.5, carBottomZ/3.5))
tireBackLeft = cylinder(radius = tireRadius, length = tireLen, color = color.black, pos = vector(carBottomX/2-tireLen*0.9, -carBottomY/2+tireRadius/2.5, -carBottomZ/3.5))
tireBackRight = cylinder(radius = tireRadius, length = tireLen, color = color.black, pos = vector(-carBottomX/2-tireLen*0.1, -carBottomY/2+tireRadius/2.5, -carBottomZ/3.5))

head = compound([headLightLeft, headLightCenter, headLightRight])
brake = compound([brakeLightCenter, brakeLightLeft, brakeLightLeftCyl, brakeLightRight, brakeLightRightCyl])

LDR_Dash = label(text = 'Dash', height = 12, pos = vector(0, carTopY*1.5, carTopZ/2))
LDR_Back = label(text = 'Back', height = 12, pos = vector(0, carTopY*1.5, -carTopZ/2))

StatusLabel = label(text = State.OFF, height = 16, pos = vector(0, carTopY*1.5+1, 0))
ErrorLabel = label(text = 'ERROR', height = 20, pos = vector(0, carTopY*1.5+1, 0), visible = False)

Threshold = 650

def LightsOn():
    head.color = onLightsColor
    brake.color = color.red
    
def LightsOff():
    head.color = offLightsColor
    brake.color = offLightsColor

def RemoveLabels():
    LDR_Dash.visible = False
    LDR_Back.visible = False

def ShowLabels(dashVal, backVal):
    LDR_Dash.text = int(dashVal)
    if(dashVal > Threshold):
        LDR_Dash.color = vector(0.7, 0.7, 0.7)
    else:
        LDR_Dash.color = color.white
    LDR_Dash.visible = True
    LDR_Back.text = int(backVal)
    if(backVal > Threshold):
        LDR_Back.color = vector(0.7, 0.7, 0.7)
    else:
        LDR_Back.color = color.white
    LDR_Back.visible = True



with serial.Serial('com4', 115200, timeout= 1) as arduino:
    time.sleep(1)
    print("Waiting for arduino...")
    while True:
        line = arduino.readline()
        line = str(line, 'utf-8').strip('\r\n')
        if 'READY' in line:
            print("Arduino ready!")
            break
    while True:
        rate(10)
        if arduino.in_waiting > 0:
            arduinoPackage = arduino.readline()
            arduinoPackage = str(arduinoPackage, 'utf-8').strip('\r\n')
            print(arduinoPackage)
            sensorReading = arduinoPackage.split()
            workMode = sensorReading[0]
            if(len(sensorReading) > 2):
                LDR_Value_Dash = int(sensorReading[1].split(':')[1])
                LDR_Value_Back = int(sensorReading[2].split(':')[1])
                LDR_Auto_Mode = sensorReading[3]
                LDR_Error = sensorReading[4]
            if(workMode == State.OFF.value):
                StatusLabel.visible = True
                RemoveLabels()
                LightsOff()
                StatusLabel.text = State.OFF.value
                StatusLabel.color = color.white
            if(workMode == State.ON.value):
                RemoveLabels()
                LightsOn()
                StatusLabel.text = State.ON.value
                StatusLabel.color = color.white
            if(workMode == State.AUTO.value):
                StatusLabel.text = State.AUTO.value
                ErrorLabel.text = LDR_Error
                ShowLabels(LDR_Value_Dash, LDR_Value_Back)
                if(LDR_Error == Fault.CRITICAL.value):
                    StatusLabel.visible = False
                    ErrorLabel.visible = True
                    LDR_Dash.color = color.red
                    LDR_Back.color = color.red
                elif(LDR_Error == Fault.DASH.value):
                    LDR_Dash.color = color.red
                    StatusLabel.visible = False
                    ErrorLabel.visible = True
                elif(LDR_Error == Fault.BACK.value):
                    LDR_Back.color = color.red
                    StatusLabel.visible = False
                    ErrorLabel.visible = True
                else:
                    StatusLabel.visible = True
                    StatusLabel.text = LDR_Auto_Mode
                    ErrorLabel.visible = False
                    ShowLabels(LDR_Value_Dash, LDR_Value_Back)
                    if(LDR_Auto_Mode == AutoMode.ON.value):
                        LightsOn()
                    elif(LDR_Auto_Mode == AutoMode.OFF.value):
                        LightsOff() 
                      