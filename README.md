# Lunch-o-meter
<img width="5712" height="4284" alt="Image" src="https://github.com/user-attachments/assets/475a4dda-d1c4-4535-8a38-7a2283facdef" />

The Lunch-o-meter is a mentometer (feedback device) specifically designed for school cafeterias. Built by Dag & Simba for their High school final project (and enhanced by their supervisor, me), this interactive system allows students to rate their daily meals and provides real-time feedback through an intuitive touchscreen interface. This repository contains detailed instructions and all necessary code for the project.

See this link for instructions on setting up your Google Sheet: https://docs.google.com/document/d/1egwhYvs6kNeSyEP7dpBwt4Gf0NozoCYfEVu8HTmKg3k/edit?usp=sharing

## Features

- **5-Point Rating System**: Students can vote using HATE, DISLIKE, NEUTRAL, LIKE, and LOVE buttons
- **Large Touchscreen Display**: 3.5" HX8357 TFT display for clear visibility in a busy cafeteria environment
- **Real-time Feedback**: Displays aggregated voting results as a bar chart
- **QR Code Integration**: Built-in QR code linking to Google Sheets for detailed vote tracking
- **WiFi Connectivity**: WPA2-Enterprise support for school network integration
- **Daily Food Updates**: Automatic daily updates of the menu item being rated
- **Batch Vote Processing**: Efficient vote aggregation before sending to cloud database
- **Swedish Language Support**: Display text in Swedish for Swedish school environment

## Installation

### Prerequisites
- Arduino IDE with ESP32 board support installed
- Required libraries (install via Arduino Library Manager):
  - Adafruit GFX Library
  - Adafruit HX8357 Driver
  - QRCodeGFX
  - U8g2 for Adafruit GFX
  - WiFi library (built-in for ESP32)
  - HTTPClient library (built-in for ESP32)
  - ArduinoJson

### Setup Steps

1. **Clone the Repository**
   ```bash
   git clone https://github.com/DagBackerud/Review-Box-Capstone-Project-.git
   ```

2. **Install Dependencies**
   - Open Arduino IDE → Sketch → Include Library → Manage Libraries
   - Search for and install the libraries listed above

3. **Configure Credentials**
   - Copy `credentials.h.example` to `credentials.h`
   - Fill in your WiFi credentials:
     - `WIFI_SSID`: Your school network name
     - `WIFI_USERNAME`: Your WPA2-Enterprise username (if applicable)
     - `WIFI_PASSWORD`: Your WiFi password
     - `SERVER_NAME`: Your backend server endpoint for vote data

4. **Upload Code**
   - Connect your ESP32 board to your computer via USB
   - Open `main` file in Arduino IDE
   - Select the correct board (ESP32 Dev Module) and port
   - Click Upload
5. **Mounting the support**
   
   -Mount the support component to the pillar.
   
   -Then fasten it with hose clamps.
<img width="284" height="378" alt="Image" src="https://github.com/user-attachments/assets/84e3c496-7039-4700-b1bf-875d8d656565" />
<img width="285" height="380" alt="Image" src="https://github.com/user-attachments/assets/ef69afe3-cb60-4dc6-8234-91fcc44e5832" />

  -Place the review box on top of the support component.
  
  -Begin screwing until both units are fitted tightly.
  
<img width="311" height="418" alt="Image" src="https://github.com/user-attachments/assets/5faa7b4b-50c9-4a2f-b34b-be60aa536bd0" />

  -Place the hose clamps attached to the review box around the pillar and fasten them.
  
<img width="255" height="344" alt="Image" src="https://github.com/user-attachments/assets/e455c1d2-64e6-408b-b4ee-22edd5e58a04" />

  -Connect the USB-C cable used for power to the microcontroller.

  -Place the lid on top of the review box and screw it into the brass inserts.
<img width="483" height="588" alt="Image" src="https://github.com/user-attachments/assets/8281c323-8130-4ce3-932f-e09ba2b0a41a" />

6. **Setting up the Google Sheet**

   -First begin by creating a new google sheet and naming it whatever you want.
<img width="1440" height="816" alt="Image" src="https://github.com/user-attachments/assets/dc37257e-dfad-4b48-aedd-c270ce8f77a5" />

   -Then paste the first row from the existing sheet into your sheet.
<img width="762" height="360" alt="Image" src="https://github.com/user-attachments/assets/189ab9cc-f8ad-46fe-8a87-5271486c9378" />
<img width="1440" height="900" alt="Image" src="https://github.com/user-attachments/assets/875cfdf6-71eb-4f8f-8354-ca647763a5b8" />
   -Select column B and then click “Format” followed by “Numbers” and select “Normal Text.
    This step makes sure that the dates are in the correct format.
<img width="1440" height="900" alt="Image" src="https://github.com/user-attachments/assets/a79e9af8-5d42-4da1-9eef-018d06bf2630" />
<img width="1440" height="900" alt="Image" src="https://github.com/user-attachments/assets/eb15a131-dda4-48d1-a7c0-583a4087df37" />


   -Write the dates and the corresponding food into Column B and  A, respectively.
<img width="492" height="272" alt="Image" src="https://github.com/user-attachments/assets/612b8bc5-d864-4aba-955e-75e29f8a5691" />

   -Click “Tillägg” and then Google Apps Script.
<img width="1047" height="266" alt="Image" src="https://github.com/user-attachments/assets/95a973af-3333-4890-bf26-5886cf4da46c" />
   -And then paste the whole code into the function.
<img width="1376" height="812" alt="Image" src="https://github.com/user-attachments/assets/14e44d97-0fa7-48fe-9750-9e24e65b35a9" />


   -Click “Implement” and then “New Implementation” 
<img width="1440" height="900" alt="Image" src="https://github.com/user-attachments/assets/0c2aaca8-76e8-43e2-adbd-342b2c82834e" />

   -Select webbapp as the type of function you are implementing.
<img width="830" height="640" alt="Image" src="https://github.com/user-attachments/assets/e53b9c4f-c423-4166-929f-6c6431019394" />
   
   -Select “all” for people who can access the function. This allows the microcontroller to communicate with the script.
<img width="764" height="600" alt="Image" src="https://github.com/user-attachments/assets/f91ef6f5-4a3d-4a52-badc-47945a845777" />

   -Approve the function to make necessary changes. And finally copy the webbapp URL into the “credentials.h” file and upload the code      to the ESP32-C5.   

 
   6.**Updating the sheet for a new year**
   -For additional years, duplicate the tab and rename it after current year. Replace rows 2 and down.
<img width="1440" height="900" alt="Image" src="https://github.com/user-attachments/assets/20289441-6dfa-4cef-8608-edadaa32d70a" />
<img width="745" height="769" alt="Image" src="https://github.com/user-attachments/assets/94127cd6-113a-4c7f-a212-652eca89e5b5" />

   -Finally move the tab so that it is the first in order and activate it, so that the QR code will show the appropriate stats.
<img width="745" height="769" alt="Image" src="https://github.com/user-attachments/assets/cdaaee51-c19d-4f91-b5bc-029512273b0b" />

   -The script update the vote counts looking up the current year.







## Parts Used

### Electronics
- **Microcontroller**: ESP32 C5 DevkitC (32-bit dual-core processor with WiFi)
- **Display**: Adafruit HX8357 3.5" 480x320 TFT Touchscreen Display
- **Communication**: SPI interface for display
- **Power Supply**: USB power adapter

### Input Controls
- STEMMA Wired Push-Buttons
- **5 Buttons**: Connected to GPIO pins (25, 7, 26, 1, 0) for voting input
- **Button Labels**: HATE, DISLIKE, NEUTRAL, LIKE, LOVE

### Physical Components
- **3D Printed Case Parts** (Fusion 360 design files included):
  - Bottom body (`Bottom body.f3d`)
  - Support body (`Support body.f3d`)
  - Top cover (`Top cover.f3d`)

### Software Components
- **Display Libraries**: For rendering graphics and text on the TFT display
- **QR Code Generator**: For creating dynamic QR codes linking to vote tracking spreadsheet
- **WiFi & HTTP**: For connecting to school network and sending/receiving data
- **JSON Processing**: For parsing menu data and formatting vote submissions
- **Time Synchronization**: NTP-based system clock for accurate date/time stamping

### Backend Integration
- **Google Sheets**: Data storage and visualization of voting results
- **Google Apps Script**: Server-side processing of vote data
- **RESTful API**: Communication between ESP32 and backend services
### Drawings

<img width="1067" height="712" alt="Image" src="https://github.com/user-attachments/assets/f16e539b-18a8-4bda-a268-31fcbfce1be7" />
<img width="1068" height="720" alt="Image" src="https://github.com/user-attachments/assets/18e81c61-f46d-46d4-9933-b95b0c5667d0" />
<img width="1064" height="710" alt="Image" src="https://github.com/user-attachments/assets/bfb71a0a-ab23-4644-b149-131384523949" />
<img width="1070" height="715" alt="Image" src="https://github.com/user-attachments/assets/55bf043c-5562-42b2-932c-d4e37c5aa1fa" />
<img width="1063" height="711" alt="Image" src="https://github.com/user-attachments/assets/ddaa1c06-6ba5-4336-b49e-a6996d4ce739" />

---

**Project Team**: Dag & Simba  
**Class**: TE23TE  
**School**: Pauliskolan, Malmö
