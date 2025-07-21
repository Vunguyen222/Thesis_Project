# Cattle Monitoring System using XIAO nRF52840 Sense

This project implements a low-power livestock monitoring system using the XIAO nRF52840 Sense. It combines location tracking, real-time activity classification, and long-range communication to monitor cattle in 
wide outdoor areas.

## 🔧 Features

- 📍 **GPS Location Tracking**  
  The system uses GPS to determine the real-time location of cattle.

- 🧠 **Activity Recognition via IMU + AI**  
  An IMU sensor is combined with a lightweight AI model to classify the cattle's activity states (e.g., walking, standing, lying) in real-time.

- 📡 **LoRa Communication**  
  Utilizes peer-to-peer (P2P) LoRa communication with a broadcasting address to transmit data to the central gateway.

- 🔄 **Energy-Efficient State Machines**  
  Designed and implemented finite state machines (FSMs) to manage GPS and LoRa power states, significantly optimizing energy consumption.

- 🔌 **Custom UART Driver**  
  Developed a minimal UART driver for the nRF52840 to handle communication with both the GPS module and LoRa transceiver at the register level.

## 🧱 Hardware Used

- **XIAO nRF52840 Sense**  
  Powered by the Nordic nRF52840, featuring built-in Bluetooth and a 6-axis IMU.

