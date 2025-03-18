# Smart Radiator Test Setup

## 1. Introduction  
The **Smart Radiator Test Setup** is designed to verify motor movement, valve control logic, and resistance detection before integrating the system into a real radiator. This ensures safe and reliable operation.  

## 2. Why Use a Test Setup?  
- Prevents potential damage to a real radiator during initial testing.  
- Fine-tunes motor movement for smooth and controlled operation.  
- Ensures the motor stops when the valve reaches its limit.  
- Identifies hardware or software issues early before real-world deployment.  

---

## 3. How to Build a Test Setup  

### **Option 1: 3D-Printed Valve Simulator**  
- Design a valve replica using a 3D modeling tool.  
- Print a test version using a 3D printer with dimensions similar to a real radiator valve.  
- Attach the servo/stepper motor to simulate real-world movement.  

This method is best for testing precise movements and physical compatibility before real deployment.  

### **Option 2: Mechanical Substitute (Simple Test Rig)**  
- Use a plastic or metal knob to mimic a radiator valve’s turning motion.  
- Mount it on a wooden or acrylic base for stability.  
- Manually block rotation at specific points to test resistance detection.  

This method is best for testing motor resistance, stopping behavior, and torque handling.  

---

## 4. Smart Radiator Test Procedure  

1. Attach the thermostat’s motor to the test valve.  
2. Command the system to adjust the valve position.  
3. Observe movement – ensure smooth and controlled rotation.  
4. Manually block the valve to check if the motor stops immediately.  
5. Test auto-calibration to see how the system maps movement to temperature change.  
6. Measure motor power usage to optimize energy efficiency.  
