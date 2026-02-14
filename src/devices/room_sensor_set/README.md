# Room Sensor Set

![](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white&style=flat)

## Requirements

When going out from room (home) I want each time some actions when proper circumstances appear. 

### Functinal Requirements

Functional requirements tell how device should behave.



### Technical Requirements

Technical reqirements describe how device is build and how executes functional requirements.



### Main Program flow

This flow describes successfull execution of device task.


## Development

Describes how device was developed and what decisions were made.

Development time of V1: 30 h.

### Problems encoutered and decisions made

### Api

### Configuration

The configuration is divided into three main sections: Wi-Fi, MQTT, and SSL. Each section defines the parameters required for network connectivity and secure communication.

#### `wifi`

Defines the credentials for connecting the device to the local wireless network.

- `ssid`: Name of the Wi-Fi network the device will connect to.
- `password`: Password for the specified Wi-Fi network.

#### `mqtt`

Specifies the settings used to connect to the MQTT broker.

- `url`: Hostname or IP address of the MQTT server.
- `port`: Network port used for the MQTT connection.
- `baseTopic`: Base MQTT topic under which the device publishes messages.
- `username`: Username for MQTT authentication.
- `password`: Password for MQTT authentication.
- `clientId`: Unique identifier used by the device when connecting to the MQTT broker.

#### `ssl`



### Assembly



#### Assembly steps


#### Uploading code and configuration


