import paho.mqtt.client as mqtt
import time
import json

BROKER_ADDRESS = "app.coreiot.io"
PORT = 1883
ACCESS_TOKEN = "nguyentruongthan"
ACCESS_USERNAME = "nguyentruongthan"


class MQTTClientHelper:
  def __init__(self):
      
    self.mqttClient = mqtt.Client(
       mqtt.CallbackAPIVersion.VERSION1,
       client_id = "than_demo_device_1")
    self.mqttClient.username_pw_set(ACCESS_USERNAME, ACCESS_TOKEN)
    self.mqttClient.connect(BROKER_ADDRESS, int(PORT), 120)

    #Register mqtt events
    self.mqttClient.on_connect = self.mqtt_connected
    self.mqttClient.on_subscribe = self.mqtt_subscribed
    self.mqttClient.on_message = self.mqtt_recv_message
    self.mqttClient.loop_start()
    self.mqttClient.on_disconnect = self.mqtt_disconnected

  def mqtt_connected(self, client, userdata, flags, rc):
      if rc == 0:
        print("Connected succesfully!!")
        client.subscribe("v1/devices/me/rpc/request/+")
      else:
        print("Connect to MQTT server failed")

  def mqtt_subscribed(self, client, userdata, mid, granted_qos):
      print("Subscribed to Topic!!!")

  def mqtt_recv_message(self, client, userdata, message):
    payload = message.payload.decode("utf-8")
    # topic = message.topic.split("/")[-1]
    topic = message.topic
    
    print(f'Receive data from topic {topic}: {payload}')


  def publish(self, message):
    print("Publish message to MQTT Server")
    self.mqttClient.publish("v1/devices/me/telemetry", message)
    
  def mqtt_disconnected(self, client, userdata, rc):  
    print(f"Disconnected from MQTT server with code {rc}")


mqttClientHelper = MQTTClientHelper()

if __name__ == "__main__":
  count = 0
  while(1):
    time.sleep(3)
    # mqttClientHelper.publish("{'temp':" + str(count) + "}")
    count += 1