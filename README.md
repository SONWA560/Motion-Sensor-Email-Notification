# Motion-Sensor-Email-Notification
IoT Motion Detection Alarm System (ESP32 + MQTT + Raspberry Pi)

Situation:
Traditional motion detection systems often rely on manual monitoring and lack real-time alert mechanisms. To address this, an automated and connected system was required to detect motion, trigger alerts, and notify users remotely through network-based communication.

Task:
Design and implement an IoT-based motion detection alarm system that could automatically sense movement, activate a buzzer, and send an email notification. The system needed to use two ESP32 microcontrollers, each equipped with a PIR motion sensor, and communicate via MQTT protocol with a Raspberry Pi broker.

Action:
Configured one ESP32 as an MQTT publisher, continuously reading data from the PIR sensor and sending motion events — including sensor ID, timestamp, and motion status — to the Raspberry Pi MQTT broker.
Integrated a buzzer alarm for immediate physical feedback when motion was detected.
Implemented Wi-Fi connectivity to ensure both ESP32 modules communicated seamlessly with the Raspberry Pi over the same network.
Extended the system to include email notifications, using Python-based backend automation to alert users in real-time upon motion detection.

Result:
The system successfully automated motion detection and alerting, reducing manual monitoring effort by over 80%. It achieved reliable real-time detection and communication between IoT devices, showcasing how ESP32, MQTT, and Raspberry Pi can be integrated for smart security and home automation applications.

Technologies Used:
Arduino (C++), ESP32, PIR Motion Sensor, Raspberry Pi, MQTT Protocol, Python (Email Automation), IoT Communication

Industry Use:
Smart home security systems, office surveillance, IoT-based intrusion detection, real-time monitoring and alert automation.
