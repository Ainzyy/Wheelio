# # 🚴‍♂️ Wheelio - Bicycle Safety Monitoring Dashboard

A real-time IoT dashboard for monitoring bicycle safety parameters including tilt sensors, lidar distance, ambient light, and acceleration data.

## 🌟 Features

- **Real-time Data Visualization**: Live Firebase integration with sensor data
- **Interactive Inclinometers**: Custom bicycle images showing real tilt orientation
- **Safety Threshold Monitoring**: Automatic warnings for dangerous conditions
- **Actuator Status Display**: Visual indicators for buzzer, warning light, and fog light
- **Demo Mode**: Standalone JavaScript simulation for presentations
- **Responsive Design**: Professional dark theme with smooth animations

## 🚀 Live Demo

- **Live Dashboard**: [View on GitHub Pages](https://yourusername.github.io/wheelio/) *(Firebase integration)*
- **Demo Dashboard**: [Test Dashboard](https://yourusername.github.io/wheelio/Test-Dashboard.html) *(JavaScript simulation)*

## 📊 Safety Rules

### Warning Conditions (Buzzer + Warning Light)
- Side tilt > 30° - Dangerous lean angle
- Front/back tilt > 9° - Steep incline/decline
- Acceleration ≥ 2 m/s² - Sudden movement detection

### Warning Light Only
- Lidar ≤ 1.50m - Obstacle detection

### Automatic Features
- Light < 1000 lux - Fog light activation (visibility aid)

## 🏗️ Project Structure

```
Wheelio/
├── index.html                    # Main dashboard (GitHub Pages default)
├── Test-Dashboard.html           # Demo dashboard with JavaScript simulation
├── README.md                     # Project documentation
├── assets/                       # Static assets
│   └── images/
│       ├── bike-front.png        # Front view bicycle image
│       └── bike-side.png         # Side view bicycle image
├── scripts/                      # Python backend scripts
│   ├── test_data_rest.py         # Firebase data generator (REST API)
│   ├── test_data.py              # Firebase data generator (Admin SDK)
│   ├── firebase_test.py          # Firebase connection testing
│   └── requirements.txt          # Python dependencies
├── archive/                      # Legacy files and backups
└── .gitignore
```

## 🛠️ Setup & Installation

### Frontend (GitHub Pages)
1. Clone the repository
2. Enable GitHub Pages in repository settings
3. Access via the GitHub Pages URL

### Backend (Python Data Generator)
```bash
# Create virtual environment
python -m venv wheelio_env
source wheelio_env/bin/activate  # On Windows: wheelio_env\Scripts\activate

# Install dependencies
pip install -r scripts/requirements.txt

# Run data generator
python scripts/test_data_rest.py
```

## 🎮 Usage

### Live Dashboard (`index.html`)
- Real-time Firebase data integration
- Click "Demo" button to switch to test mode
- Requires Firebase configuration for live data

### Test Dashboard (`Test-Dashboard.html`)
- Standalone JavaScript simulation
- Interactive demo controls
- Perfect for presentations and testing
- Click "Live" button to return to main dashboard

### Demo Controls
- **Start/Stop**: Control data generation
- **Scenarios**: Cycle through predefined test cases
- **Random**: Generate realistic random sensor data

## 🔧 Configuration

### Firebase Setup
Update Firebase configuration in `index.html`:
```javascript
const firebaseConfig = {
    apiKey: "your-api-key",
    authDomain: "your-project.firebaseapp.com",
    databaseURL: "your-database-url",
    projectId: "your-project-id",
};
```

### Python Scripts
Configure Firebase URL in `scripts/test_data_rest.py`:
```python
FIREBASE_URL = "https://your-database-url/test_data1"
```

## 🎨 Visual Features

- **Custom Bike Visualizers**: Real bicycle images that tilt with sensor data
- **Directional Labels**: Clear LEFT/RIGHT and BACK/FRONT indicators
- **Smooth Animations**: Professional transitions and hover effects
- **Color-coded Status**: Blue for demo mode, red for live mode
- **Responsive Grid Layout**: Adapts to different screen sizes

## 🔄 Data Flow

1. **Sensors** → Generate tilt, lidar, light, acceleration data
2. **Python Script** → Apply safety rules and upload to Firebase
3. **Dashboard** → Real-time visualization with threshold monitoring
4. **Actuators** → Visual feedback for buzzer, warning light, fog light

## 📱 Browser Compatibility

- Chrome/Chromium (recommended)
- Firefox
- Safari
- Edge

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test with both live and demo modes
5. Submit a pull request

## 📄 License

This project is part of the KHS Capstone program for embedded systems and bicycle safety monitoring.

## 🚴‍♂️ About Wheelio

Wheelio is an IoT bicycle safety system designed to monitor critical parameters and provide real-time feedback to cyclists. The dashboard provides an intuitive interface for understanding bicycle orientation, environmental conditions, and potential safety hazards.