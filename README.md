# Smart-Home-System-Setting-the-Temperature
Smart home system that we made at the IoT Course in Bucharest in July 2022 (read the description for more information)
Bucharest IoT course completion project
members
Eren Yurtsever (Turkey)
Dat Lee (Vietnam)
Emmely Vigsoe (Denmark)

ESP32_1:
It is responsible for detecting the host using NFC technology and opening the door
It saves every Nfc movement in google tables and keeps the guest list
It communicates with other ESP32S via Wifi to enable them to be activated


ESP32_2:
It starts working with the order it receives from ESP32_1 via Wifi
Measures room temperature and humidity
He is responsible for the operation of the Fan in ESP32_3 via Wifi

ESP32_3
It starts the fan with the command it receives from ESP32_2 via Wifi
