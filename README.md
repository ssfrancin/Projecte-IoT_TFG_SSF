# Projecte-IoT_TFG_SSF
Projecte IoT


IOT SYSTEM FOR VITAL SIGNS MONITORING
This thesis describes an interconnected device system based on IoT technology to measure a series of vital signs and monitor data in real time from a control panel located on a local network or in the cloud, which visually represents the information processed in through graphs with the ability to detect situations of risk to people's health.
The impact of this project is to contribute to the community at large with a pilot prototype capable of acting as a tool to prevent the spread of pandemics that alter a person’s vital signs.
This is the first public release. Version 0.1 (June 2020)


REQUIREMENTS
Hardware:
- Raspberry Pi 3 Model B+
- NodeMCU Lua Lolin V3
- MAX30102
- DHT11

Software:
- Arduino IDE
- OS Raspbian
- Grafana
- InfluxDB
- MQTT Broker - Mosquitto
- RealVNC


INSTALL
1)Install InfluxDB, Grafana and MQTT Broker on Raspberry Pi:
-InfluxDB
curl -sL https://repos.influxdata.com/influxdb.key | sudo apt-key add-
echo "deb https://repos.influxdata.com/debian stretch stable" | sudo tee/etc/apt/sources.list.d/influxdb.list							
sudo apt Update
sudo apt install influxdb  								
sudo pip3 install influxdb						
sudo pip3 install paho-mqtt						
sudo systemctl enable influxdb						
sudo systemctl start influxdb	

-Grafana
wget https://github.com/fg2it/grafana-on-raspberry/releases/download/v5.1.4/grafana_5.1.4_armhf.deb		
sudo dpkg –i grafana_5.1.4_armhf.deb					
sudo systemctl enable grafana-server					
sudo systemctl start grafana-server
						
-MQTT Broker - Mosquitto
sudo apt install -y mosquitto mosquitto-clients			
sudo systemctl enable mosquitto.service

2)Place the "RaspberryPi" folder on the RaspberryPi

3)Install Arduino IDE on a personal computer or on Raspberry Pi, and upload the files from "NodeMCU" folder to the microcontroller NodeMCU.

4)Create a Grafana user, and edit the Grafana JSON from the code provided in the "json_grafana.txt".

5)Connect all devices, ensure to enable wifi to Rpi and NodeMCU. Now everything is settled.

6)To run the demo, type on the Rpi terminal:
python3 mqtt_to_influxdb.py


DOCUMENTATION
Documentation and software are available in the following links:

OS RaspberryPi: https://www.raspberrypi.org/downloads/raspberry-pi-os/
RealVCN: https://www.realvnc.com/en/connect/download/viewer/
InfluxDB: https://www.influxdata.com/products/influxdb-overview/
Grafana: https://grafana.com/oss/grafana/
Ubidots: https://industrial.ubidots.com/accounts/signup_industrial/


BRIEF PROJECT DESCRIPTION
This project develops an IoT system end to end.
NodeMCU collects from the sensors 5 different measures that are transmitted to the RaspberryPi via wireless MQTT protocol in same network.
Raspberry Pi shows the data in a front-end (Grafana) and allows to configure alerts and notifications.
Raspberry Pi also sends the information to Ubidots via Internet in order to publish all metrics in public.


CONTACT
samuel.sanchez.francin@estudiant.upc.edu
antoni.oller@upc.edu
