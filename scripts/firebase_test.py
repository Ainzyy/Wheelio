#!/usr/bin/env python3
"""
Simple Firebase Connection Test
This script tests Firebase connectivity step by step
"""

import firebase_admin
from firebase_admin import credentials, db
import json

def test_firebase_connection():
    """Test Firebase connection with different methods"""
    
    print("🔧 Firebase Connection Debug Test")
    print("=" * 50)
    
    # Firebase configuration
    firebase_config = {
        "databaseURL": "https://wheelio-0o-default-rtdb.asia-southeast1.firebasedatabase.app",
        "projectId": "wheelio-0o",
    }
    
    print(f"📊 Testing connection to: {firebase_config['databaseURL']}")
    
    # Method 1: Try Application Default Credentials
    print("\n🔍 Method 1: Application Default Credentials")
    try:
        if not firebase_admin._apps:
            cred = credentials.ApplicationDefault()
            firebase_admin.initialize_app(cred, {
                'databaseURL': firebase_config["databaseURL"]
            })
        
        ref = db.reference('test_connection')
        test_data = {"test": True, "timestamp": db.ServerValue.TIMESTAMP}
        
        print("📤 Attempting to write test data...")
        ref.set(test_data)
        print("✅ SUCCESS: Data written successfully!")
        
        print("📥 Attempting to read test data...")
        result = ref.get()
        print(f"✅ SUCCESS: Data read: {result}")
        
        return True
        
    except Exception as e:
        print(f"❌ FAILED: {e}")
        print("📝 This is expected if you don't have Firebase CLI set up")
    
    # Method 2: Try with REST API approach (read-only test)
    print("\n🔍 Method 2: REST API Test (Read-only)")
    try:
        import requests
        url = f"{firebase_config['databaseURL']}/test_data1.json"
        response = requests.get(url)
        
        if response.status_code == 200:
            data = response.json()
            print(f"✅ SUCCESS: Can read from database")
            print(f"📊 Current data keys: {list(data.keys()) if data else 'No data'}")
            return True
        else:
            print(f"❌ FAILED: HTTP {response.status_code}")
            
    except Exception as e:
        print(f"❌ FAILED: {e}")
    
    print("\n📋 DIAGNOSIS:")
    print("   The Firebase Admin SDK requires authentication.")
    print("   Your web app uses client-side authentication, but")
    print("   the Python script needs server-side authentication.")
    print("\n🔧 SOLUTIONS:")
    print("   1. Use Firebase CLI authentication")
    print("   2. Download service account key")
    print("   3. Use REST API with database rules")
    
    return False

if __name__ == "__main__":
    test_firebase_connection()
