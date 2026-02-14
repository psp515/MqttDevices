#ifndef MQTT_TRANSPORT_H
#define MQTT_TRANSPORT_H

#include "transport.h"

using namespace mqttdevices::transport;

namespace mqttdevices::transport::mqtt {

    class MqttMessage final : public TransportMessage {

    };

    class MqttTransport final : public Transport {
    
    };
}

#endif