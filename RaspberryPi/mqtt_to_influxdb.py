import re
import requests
import time
from typing import NamedTuple
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient

INFLUXDB_ADDRESS = 'localhost'
INFLUXDB_USER = 'mqtt'
INFLUXDB_PASSWORD = 'mqtt'
INFLUXDB_DATABASE = 'maindb'

MQTT_ADDRESS = 'localhost'
MQTT_USER = ''
MQTT_PASSWORD = ''
MQTT_TOPIC = 'nodemcu/+/+'
MQTT_REGEX = 'nodemcu/([^/]+)/([^/]+)'
MQTT_CLIENT_ID = 'MQTTInfluxDBBridge'

TOKEN = "BBFF-fwIR4A6QMrHOhBdOodDu5zClIyo5ag"
DEVICE_LABEL = "Sam_Rpi" 
VARIABLE_LABEL_1 = "humidity"  
VARIABLE_LABEL_2 = "temperature"  
VARIABLE_LABEL_3 = "heartrate" 
VARIABLE_LABEL_4 = "oxygen"
VARIABLE_LABEL_5 = "bodytemp"


influxdb_client = InfluxDBClient(INFLUXDB_ADDRESS, 8086, INFLUXDB_USER, INFLUXDB_PASSWORD, None)

class SensorData(NamedTuple):
    location: str
    measurement: str
    value: float


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)

def _parse_mqtt_message(topic, payload):
    match = re.match(MQTT_REGEX, topic)
    if match:
        location = match.group(1)
        measurement = match.group(2)
        if measurement == 'status':
            return None
        return SensorData(location, measurement, float(payload))
    else:
        return None

def _send_sensor_data_to_influxdb(sensor_data):
    json_body = [
        {
            "measurement": sensor_data.measurement,
            "tags": {
                "location": sensor_data.location
            },
            "fields": {
                "value": sensor_data.value
            }
        }
    ]
    influxdb_client.write_points(json_body)



def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' ' + str(msg.payload))
    sensor_data = _parse_mqtt_message(msg.topic, msg.payload.decode('utf-8'))
        
    if sensor_data is not None:
        _send_sensor_data_to_influxdb(sensor_data)
    
    if msg.topic.find(VARIABLE_LABEL_1) is not -1:
        ubidots_payload = {VARIABLE_LABEL_1:msg.payload.decode('utf-8')}
    
    elif msg.topic.find(VARIABLE_LABEL_2) is not -1:
        ubidots_payload = {VARIABLE_LABEL_2:msg.payload.decode('utf-8')}
    
    elif msg.topic.find(VARIABLE_LABEL_3) is not -1:
        ubidots_payload = {VARIABLE_LABEL_3:msg.payload.decode('utf-8')}
    
    elif msg.topic.find(VARIABLE_LABEL_4) is not -1:
        ubidots_payload = {VARIABLE_LABEL_4:msg.payload.decode('utf-8')}
   
    elif msg.topic.find(VARIABLE_LABEL_5) is not -1:
        ubidots_payload = {VARIABLE_LABEL_5:msg.payload.decode('utf-8')}
    
    
    post_request(ubidots_payload)



def post_request(payload):

    try: 
        # Creates the headers for the HTTP requests
        url = "http://industrial.api.ubidots.com"
        url = "{}/api/v1.6/devices/{}".format(url, DEVICE_LABEL)
        headers = {"X-Auth-Token": TOKEN, "Content-Type": "application/json"}
    except Exception as e:
        print(e)
    # Makes the HTTP requests
    status = 400
    attempts = 0
    
    while (status >= 400 and attempts <= 5):
        
        req = requests.post(url=url, headers=headers, json=payload)
        
        status = req.status_code
        attempts += 1
        time.sleep(1)
    
    # Processes results
    if status >= 400:
        print("[ERROR] Could not send data after 5 attempts, please check \
            your token credentials and internet connection")
        return False

    print("[INFO] request made properly, your device is updated")
    
    return True

def _init_influxdb_database():
    databases = influxdb_client.get_list_database()
    if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
        influxdb_client.create_database(INFLUXDB_DATABASE)
    influxdb_client.switch_database(INFLUXDB_DATABASE)

def main():
    _init_influxdb_database()

    mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_forever()


if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()


