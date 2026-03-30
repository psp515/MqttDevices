# Room Sensor Set

![](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white&style=flat)

Simple Arduino device for monitoring base informations about rooms.

## Use Cases

### Leaving Room

When I leave room I want light to automatically turn off because i forget it frequently.

### Temperature

Temperature in my room is frequently low and quickly falls. I want to understand how much temperature falls during the night and display it.

## Requirements

Functional requirements tell how device should behave and technical how device deliveres requirements.

- compatible with [Senswave](https://www.senswave.net/) app
- sends temperature in room
- monitors human presence in room and notifies appearing and disappering
- device must notify if it is online or offline

### Functinal Requirements

Functional requirements tell how device should behave.

### Technical Requirements

- DHT22 for humidity and temperature recording
- Human Static presence sensor for moniroting if there is anyone in the room
- use Raspberry Pi Pico W board
- Use MQTT Connection for udpates to and from device

### Main Program flow

This flow describes successfull execution of device task.

- read configuration
- connect to wifi
- setup security
- connect to mqtt
- loop
    - check wifi
    - check mqtt connection
    - update time
    - check temperature and humidity
    - chekc human presence

## Development

Describes how device was developed and what decisions were made.

### Problems encoutered and decisions made

- My room fits perfectly sensor detection area so I don't need to update distance properties.
- I am monitoring just human presence / absence so I can use build in pin for monitoring those
- Sensor will be placed on flat area and can use power source so batteries are not needed.

## API

### Event - Device Online / Offline

Topic: `<mqtt:baseTopic>/status`, example: `device/room/sensor/set/1/status`

Event tells if device is working and connected to the mqtt server. Uses for that mqtt mechanisms retained message and last will.
Payload is send as normal text.

```
[true|false]
```

### Event - Human Presence/Absence

Topic: `<mqtt:baseTopic>/presence`, example: `device/room/sensor/set/1/presence`

On button press device generates one evnet on configured topic `<mqtt:baseTopic>/presence`, example: `device/room/sensor/set/1/presence`.
Payload is send as normal text.

```
[true|false]
```

#### Option - Human Presence/Absence Update Status

Topic: `<mqtt:baseTopic>/presence/status`, example: `device/room/sensor/set/1/presence/status`

It is possible to manually disable sending updates of human presence and absence in room.
Payload is send as normal text.

```
[true|false]
```

### Event - Temperature/Humidity Update

Topic: `<mqtt:baseTopic>/environment`, example: `device/room/sensor/set/1/environment`

When temperature changes at least for 0.5 degree event is send or periodically every 15 minutes.  
Payload is send as json.

```json
{
    "temp": 24.5, 
    "tempUnit": "°C", 
    "hum": 43.2, 
    "humUnit": "%"
}
```

#### Option - Temperature/Humidity Update Status

Topic: `<mqtt:baseTopic>/environment/status`, example: `device/room/sensor/set/1/environment/status`

It is possible to manually disable sending updates of humidity and temperature in room.
Payload is send as normal text.

```
[true|false]
```

## Configuration

The configuration is divided into three main sections: Wi-Fi, MQTT, and NTP. Each section defines the parameters required for network connectivity and secure communication.

#### `wifi`

Defines the credentials for connecting the device to the local wireless network.

- `ssid`: Name of the Wi-Fi network the device will connect to.
- `password`: Password for the specified Wi-Fi network.


#### `ntp`

Section for defining how frequently device should refresh its device time and what servers to use.

- `server1`: Server 1 for updating time.
- `server2`: Server 2 for updating time.
- `updateIntervalMs`: Miliseconds between time updates. 

#### `mqtt`

Specifies the settings used to connect to the MQTT broker.

- `url`: Hostname or IP address of the MQTT server.
- `port`: Network port used for the MQTT connection.
- `baseTopic`: Base MQTT topic under which the device publishes messages.
- `username`: Username for MQTT authentication.
- `password`: Password for MQTT authentication.
- `clientId`: Unique identifier used by the device when connecting to the MQTT broker.
- `ssl`: section for security properties
- `ssl:enabled`: section for enabling/disabling ssl certificate check
- `ssl:certFile`: certificate filename.

## Assembly

TODO

### Assembly steps

TODO

### Uploading code and configuration

Easiest way is to configure [Arduiono IDE](https://www.arduino.cc/en/software/) to compile and upload program to the board.
That's what I assume you will do.

- open `src/devices/room_sensor_set`
    - update properties in `appsettings.json` according to configuration section and for your needs
    - copy contents of `appsettings.json`
- open `examples/filesystem.ino`
    - upload certificate by running the program (optional) (hivemq certificate for started and basics accounts already configured)
    - paste `appsettings.json` conent into `fileContent` property
    - change `filename` to `appsettings.json`
    - run program to upload configuration
- open `src/devices/room_sensor_set`
    - run program to upload proper uf2.file

TODO: Wiring Schematic

## Possible Enhancments

- [allowing Over the air update](https://randomnerdtutorials.com/esp8266-nodemcu-ota-over-the-air-arduino/)
- web server for simple configuration update
- Dynamically update Static human sensor configuration through rx tx
- use simpler device like ESP01 - rpi is too much power
- adding support for a IR pilot for configuring options in room 