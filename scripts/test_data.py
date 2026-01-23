#!/usr/bin/env python3
"""
Test Data Generator for Wheelio Dashboard
Generates realistic sensor data and uploads to Firebase RTDB every 1 second
"""

import firebase_admin
from firebase_admin import credentials, db
import time
import random
import math
from datetime import datetime

# Firebase configuration
firebase_config = {
    "apiKey": "AIzaSyDg9Le8N-x1v30_ArHJWQGoEIoNdUXStjI",
    "authDomain": "wheelio-0o.firebaseapp.com",
    "databaseURL": "https://wheelio-0o-default-rtdb.asia-southeast1.firebasedatabase.app",
    "projectId": "wheelio-0o",
}

def initialize_firebase():
    """Initialize Firebase Admin SDK"""
    try:
        # For testing purposes, we'll try to initialize without service account
        # This will work if you have Firebase CLI installed and authenticated
        if not firebase_admin._apps:
            try:
                # Try to use default credentials first (if Firebase CLI is set up)
                cred = credentials.ApplicationDefault()
                firebase_admin.initialize_app(cred, {
                    'databaseURL': firebase_config["databaseURL"]
                })
                print("✅ Using Application Default Credentials")
            except Exception as default_error:
                print(f"❌ Application Default Credentials failed: {default_error}")
                print("📝 Trying alternative method...")
                
                # Alternative: Use a minimal service account approach
                # Note: This won't work with dummy credentials, but shows the structure
                try:
                    # You would need to replace this with actual service account JSON
                    service_account_info = {
                        "type": "service_account",
                        "project_id": "wheelio-0o",
                        "private_key_id": "your_private_key_id",
                        "private_key": "your_private_key",
                        "client_email": "your_service_account_email",
                        "client_id": "your_client_id",
                        "auth_uri": "https://accounts.google.com/o/oauth2/auth",
                        "token_uri": "https://oauth2.googleapis.com/token"
                    }
                    cred = credentials.Certificate(service_account_info)
                    firebase_admin.initialize_app(cred, {
                        'databaseURL': firebase_config["databaseURL"]
                    })
                    print("✅ Using Service Account Credentials")
                except Exception as service_error:
                    print(f"❌ Service Account Credentials failed: {service_error}")
                    print("🔧 Running in simulation mode - no actual Firebase uploads")
                    return None
        
        return db.reference('test_data1')
    except Exception as e:
        print(f"❌ Firebase initialization error: {e}")
        print("📋 To fix this, you need to:")
        print("   1. Install Firebase CLI: npm install -g firebase-tools")
        print("   2. Login: firebase login")
        print("   3. Or download service account key from Firebase Console")
        return None

def generate_sensor_data():
    """Generate realistic sensor data with some variation"""
    # Generate tilt values (normally around 0, but can vary)
    tilt_side = random.uniform(-45, 45)  # Side tilt (roll) -45 to +45 degrees
    tilt_fb = random.uniform(-15, 15)    # Front/back tilt (pitch) -15 to +15 degrees
    
    # Generate other sensor values
    lidar = random.uniform(0.5, 5.0)     # Lidar distance 0.5 to 5.0 meters
    light = random.uniform(0, 2000)      # Light sensor 0 to 2000 lux
    accel_x = random.uniform(-10, 10)    # Acceleration -10 to +10 m/s²
    
    return {
        "sensors": {
            "tilt_side": round(tilt_side, 5),
            "tilt_fb": round(tilt_fb, 5),
            "lidar": round(lidar, 5),
            "light": round(light, 5),
            "accel_x": round(accel_x, 5)
        },
        "timestamp": db.ServerValue.TIMESTAMP  # Use Firebase server timestamp
    }

def calculate_actuators_and_warnings(sensor_data):
    """
    Calculate actuator states and warning messages based on sensor thresholds
    
    Warning Rules:
    - Side tilt > 30 degrees: buzzer + warning light
    - Front/back tilt > 9 degrees: buzzer + warning light  
    - Acceleration >= 2 m/s²: buzzer + warning light
    - Lidar <= 1.50 meters: warning light only
    
    Automatic Features:
    - Light < 1000 lux: fog light (automatic, not a warning)
    """
    sensors = sensor_data["sensors"]
    
    # Initialize actuators
    buzzer = False
    warning_light = False
    fog_light = False
    warnings = []
    
    # Check side tilt (roll)
    if abs(sensors["tilt_side"]) > 30:
        buzzer = True
        warning_light = True
        warnings.append("Side tilt warning")
    
    # Check front/back tilt (pitch)
    if abs(sensors["tilt_fb"]) > 9:
        buzzer = True
        warning_light = True
        warnings.append("Pitch tilt warning")
    
    # Check acceleration
    if abs(sensors["accel_x"]) >= 2.0:
        buzzer = True
        warning_light = True
        warnings.append("High acceleration warning")
    
    # Check lidar distance
    if sensors["lidar"] <= 1.50:
        warning_light = True
        warnings.append("Obstacle detected")
    
    # Check light level (fog light activates automatically, not a warning)
    if sensors["light"] < 1000:
        fog_light = True
        # Note: Fog light activation is not a warning condition
    
    # Create warning message
    if warnings:
        if len(warnings) == 1:
            warning_message = warnings[0]
        else:
            warning_message = " + ".join(warnings)
    else:
        warning_message = ""
    
    # Add actuators to sensor data
    sensor_data["actuators"] = {
        "buzzer": buzzer,
        "warning_light": warning_light,
        "fog_light": fog_light
    }
    
    sensor_data["warning"] = warning_message
    
    return sensor_data

def create_test_scenarios():
    """Create specific test scenarios to demonstrate different conditions"""
    scenarios = [
        # Normal operation
        {
            "tilt_side": 5.0,
            "tilt_fb": 2.0,
            "lidar": 3.0,
            "light": 1500,
            "accel_x": 1.2
        },
        # Side tilt warning
        {
            "tilt_side": 35.0,
            "tilt_fb": 3.0,
            "lidar": 2.5,
            "light": 1200,
            "accel_x": 2.1
        },
        # Pitch tilt warning
        {
            "tilt_side": 8.0,
            "tilt_fb": 12.0,
            "lidar": 2.0,
            "light": 1100,
            "accel_x": 3.5
        },
        # Obstacle warning
        {
            "tilt_side": 5.0,
            "tilt_fb": 3.0,
            "lidar": 1.2,
            "light": 1300,
            "accel_x": 0.8
        },
        # Low light (fog light)
        {
            "tilt_side": 2.0,
            "tilt_fb": 1.0,
            "lidar": 2.8,
            "light": 800,
            "accel_x": 1.0
        },
        # High acceleration warning
        {
            "tilt_side": 5.0,
            "tilt_fb": 3.0,
            "lidar": 2.5,
            "light": 1200,
            "accel_x": 3.5
        },
        # Multiple warnings
        {
            "tilt_side": 40.0,
            "tilt_fb": 11.0,
            "lidar": 1.0,
            "light": 500,
            "accel_x": 8.5
        }
    ]
    return scenarios

def run_test_data_generator():
    """Main function to run the test data generator"""
    print("🚀 Starting Wheelio Test Data Generator")
    print("📊 Uploading to Firebase path: test_data1")
    print("⏱️  Upload interval: 1 second")
    print("\n📋 Warning Rules:")
    print("   • Side tilt > 30°: Buzzer + Warning Light")
    print("   • Front/back tilt > 9°: Buzzer + Warning Light")
    print("   • Acceleration ≥ 2 m/s²: Buzzer + Warning Light")
    print("   • Lidar ≤ 1.50m: Warning Light")
    print("\n💡 Automatic Features:")
    print("   • Light < 1000 lux: Fog Light (visibility aid)")
    print("\n" + "="*50)
    
    # Initialize Firebase
    ref = initialize_firebase()
    
    if ref is None:
        print("❌ Firebase initialization failed. Running in simulation mode...")
        simulate_only = True
    else:
        print("✅ Firebase initialized successfully!")
        simulate_only = False
    
    # Get test scenarios
    scenarios = create_test_scenarios()
    scenario_index = 0
    use_scenarios = True
    
    try:
        iteration = 0
        while True:
            iteration += 1
            
            # Decide whether to use scenario or random data
            if use_scenarios and scenario_index < len(scenarios):
                # Use predefined scenario
                scenario = scenarios[scenario_index]
                sensor_data = {
                    "sensors": {
                        "tilt_side": scenario["tilt_side"],
                        "tilt_fb": scenario["tilt_fb"],
                        "lidar": scenario["lidar"],
                        "light": scenario["light"],
                        "accel_x": scenario["accel_x"]
                    },
                    "timestamp": db.ServerValue.TIMESTAMP
                }
                scenario_index += 1
                print(f"\n📋 Iteration {iteration} - Using Scenario {scenario_index}")
            else:
                # Use random data
                sensor_data = generate_sensor_data()
                print(f"\n🎲 Iteration {iteration} - Random Data")
            
            # Calculate actuators and warnings
            complete_data = calculate_actuators_and_warnings(sensor_data)
            
            # Display data
            sensors = complete_data["sensors"]
            actuators = complete_data["actuators"]
            warning = complete_data["warning"]
            
            print(f"📊 Sensors:")
            print(f"   Tilt Side: {sensors['tilt_side']:>8.2f}° | Tilt FB: {sensors['tilt_fb']:>8.2f}°")
            print(f"   Lidar:     {sensors['lidar']:>8.2f}m | Light:   {sensors['light']:>8.1f} lux")
            print(f"   Accel X:   {sensors['accel_x']:>8.2f} m/s²")
            
            print(f"🔧 Actuators:")
            print(f"   Buzzer: {'🔊 ON ' if actuators['buzzer'] else '🔇 OFF'} | "
                  f"Warning: {'⚠️  ON ' if actuators['warning_light'] else '✅ OFF'} | "
                  f"Fog: {'🌫️  ON ' if actuators['fog_light'] else '☀️  OFF'}")
            
            if warning:
                print(f"⚠️  Warning: {warning}")
            else:
                print("✅ Status: All systems normal")
            
            # Upload to Firebase
            if not simulate_only:
                try:
                    ref.push(complete_data)
                    print("📤 Data uploaded to Firebase successfully")
                except Exception as e:
                    print(f"❌ Firebase upload error: {e}")
            else:
                print("💻 Simulation mode - data not uploaded")
            
            # Wait 1 second
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\n\n🛑 Test data generator stopped by user")
        print("👋 Goodbye!")

if __name__ == "__main__":
    run_test_data_generator()