# Smart Radiator Thermostat Project

## 1. Project Description

The goal of this project is to develop a **smart thermostat** that can automatically control a traditional radiator.  
The system will:
- Measure the **room temperature** using **two sensors** (one in the room and one near the battery) to calculate an **average temperature**.
- Allow users to **set a target temperature**.
- Adjust the **radiator valve** using a motor.
- Display **real-time temperature data** on a screen.
- Enable **scheduled temperature changes** at specific times.

### **Challenges**
- **Universal Motor Attachment:** The motor mechanism must be adaptable to different radiator valve types.
- **Auto-Calibration:** The system must learn how much to turn the valve to achieve the desired temperature.
- **Real-Time Safety:** If the **valve reaches its limit**, the motor must **immediately stop** to prevent mechanical damage.

---

## 2. Required Hardware & Costs

| Component | Description | Price (EUR) |
|-----------|------------|-------------|
| **Microcontroller** | LilyGO TTGO T3 LoRa32 868MHz V1.6.1 ESP32 | 21.50 |
| **Temperature Sensors** | DHT22 or DS18B20 (One near the radiator, one near the battery) | 5.00 - 10.00 |
| **Motor** | Servo/Stepper Motor (for valve control) | 5.00 - 15.00 |
| **Display** | OLED or LCD Screen | 10.00 - 20.00 |
| **Input** | Buttons/Rotary Encoder (for setting temp & time) | 3.00 - 7.00 |
| **Power Supply** | 5V/12V adapter or battery | 5.00 - 15.00 |
| **Mounting** | 3D-printed or adjustable mount for the radiator valve | 10.00 - 30.00 |
| **Miscellaneous** | Breadboard, jumper wires, resistors, etc. | 10.00 |

**Estimated Total Cost:** _50 - 120 EUR_ (depending on component choices)

---

## 3. Explanation of the Project

### **1. Temperature Measurement & Display**
- The system **reads temperature data** from **two sensors**:
  - One placed **in the room**.
  - One placed **near the battery**.
- The system **calculates an average temperature** for more accurate readings.
- The temperature is displayed on an **OLED/LCD screen**.

### **2. User Input & Configuration**
- The user can **set the target temperature** using **buttons or a rotary encoder**.
- A **timer function** allows scheduling changes (e.g., _"Set temperature to 22°C at 8:00 AM"_).

### **3. Radiator Valve Control**
- A **motor** is attached to the radiator valve and adjusts based on temperature differences.
- **Auto-calibration** detects the valve's motion range and maps it to temperature changes.
- **Real-Time Safety Feature:**  
  - If the radiator **cannot turn further**, the system will **immediately stop the motor** to prevent damage.

### **4. Communication & Processing**
- The **ESP32 microcontroller** processes **temperature data** and **user inputs**.
- It **controls the motor** to maintain the desired temperature.

### **5. Power & Hardware Integration**
- Powered via a **5V/12V adapter or a battery**.
- All components will be **mounted in a casing** attached to the radiator.

---

## 4. Key Challenges & Solutions

- **Universal Fit:**  
  - The motor attachment must be **adjustable** for different radiator types.  
  - Possible solutions: **3D-printed mounts** or **adjustable clamps**.

- **Auto-Calibration:**  
  - The system must determine how much to turn the valve to reach the target temperature.  
  - May require an **initial learning phase**.

- **Power Efficiency:**  
  - If **battery-powered**, the system should **enter low-power mode** when inactive.

- **Real-Time Safety Mechanism:**  
  - If the motor reaches its **movement limit**, it will **immediately stop** to prevent damage.
  - The system will detect **resistance** and halt operation.

---

## 5. Learning Outcomes

By completing this project, We will gain experience in:

- **Hardware-Software Integration:** Connecting sensors, motors, and displays in an embedded system.
- **Software Development:** Handling user input and scheduling functions.
- **Algorithm Implementation:** Developing **auto-calibration logic** for precise control.
- **Mechanical Design:** Creating mounting solutions for electronic components.
- **Real-Time Control:** Implementing a safety mechanism that stops movement when resistance is detected