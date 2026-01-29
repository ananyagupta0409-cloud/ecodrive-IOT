# ECO DRIVE – Smart Vehicular Emission Monitoring System

## Overview
Vehicular exhaust emissions are a major contributor to urban air pollution, leading to respiratory illnesses, cardiovascular problems, and poor air quality. Current regulatory practices rely on periodic emission testing at authorized centers, which fail to provide continuous monitoring and may miss intermittent pollution spikes or post-test tampering.

**ECO DRIVE** proposes a portable, low-cost IoT-based solution for real-time monitoring of vehicle emission-related parameters. The system continuously captures environmental and gas concentration data, processes it through an IoT gateway, and streams it to cloud services for visualization and analysis through mobile and dashboard applications.

This prototype demonstrates how continuous monitoring can enable better emission tracking and awareness compared to traditional periodic testing methods.

---

## System Architecture
The system follows a classic three-layer IoT architecture:

### 1. Sensing Layer
- **MQ-135 Gas Sensor** – Detects harmful gases and smoke typically associated with vehicular emissions.
- **DHT22 Sensor** – Measures ambient temperature and humidity, which influence emission dispersion and sensor behavior.

### 2. Processing & Communication Layer
- **ESP32 Microcontroller**
  - Reads analog and digital sensor data
  - Computes an emission score from raw readings
  - Handles Wi-Fi connectivity
  - Synchronizes timestamps using NTP
  - Sends structured JSON data to the cloud

### 3. Application Layer
- **Firebase Realtime Database** for cloud data storage and synchronization.
- **Flutter Mobile Application** for real-time visualization, alerts, and historical logs.
- **Arduino IoT Cloud Dashboard** for monitoring and analytics.

### Data Flow

