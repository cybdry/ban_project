#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
from flask import Flask, render_template
from flask_socketio import SocketIO
from firebase_admin import credentials, initialize_app, db
import json
import time
from math import sqrt

app = Flask(__name__,static_folder='static',)
socket_io = SocketIO(app)

# Initialize firebase
cred = credentials.Certificate('./credential.json')
initialize_app(cred, {
    'databaseURL': 'https://bannewproject-default-rtdb.europe-west1.firebasedatabase.app/'
})


# Set up the Firebase database reference and attach the listener
reference_database = db.reference("/sensor")

def calculate_linear_speed(x,y,z):
    return sqrt(int(x)**2 + int(y)**2 + int(z)**2)

def sensor_fusion(bpm, speed)->bool:
    bpm_threshold = 120
    speed_threshold = 20

    if(bpm > bpm_threshold and speed > speed_threshold):
        return True
    else:
        return False
@app.route("/")
def index():
    return render_template('index.html')

# Additional route to get the initial data
@app.route("/get_data")
def get_data_route():
    data = reference_database.get();
    bpm = data["heart"]["rate"]
    linear_speed = calculate_linear_speed(data["acc"]["x"],data["acc"]["y"],data["acc"]["z"]);
    print("BPM: ",bpm)
    print("Linear speed: ",linear_speed)
    alert_ref = reference_database.child('alert')
    if(sensor_fusion(bpm,linear_speed)):
        reference_database.update({"alert":1}) 
    else:

        reference_database.update({"alert":0}) 

    return json.dumps(reference_database.get())

if __name__ == "__main__":
    app.run(debug=True)

