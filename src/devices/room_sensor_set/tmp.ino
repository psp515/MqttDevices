#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <Arduino.h>
#include <string.h>

namespace mqttdevices::transport {

    class TransportMessage {
    public:
      TransportMessage() : payload(nullptr), id(nullptr) {}
      TransportMessage(const char* _id const char* _payload) : payload(_payload), id(_id) {}
      const char* getId() const { return id; }
      const char* getPayload() const { return payload; }
      
    private: 
      char* payload;
      char* id;  
    };

    class Transport {
    public:
        virtual void start();
        virtual bool send(Message message);
        virtual void receive();
        virtual void observe(const char* id, const char* payload);
    };
}

#endif

#ifndef MQTT_TRANSPORT_H
#define MQTT_TRANSPORT_H

namespace mqttdevices::transport::mqtt {

    class MqttMessage final : public TransportMessage {
    public:
            

    private: 
        char* payload;
        char* id;
        
    };

    class MqttTransport final : public Transport {
    public:
    };
}

#endif