#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <Arduino.h>
#include <string.h>

namespace smartdevices::transport {

    class TransportMessage {
    public:
      TransportMessage() : payload(nulc:\Users\kolbe\Desktop\test.inolptr), path(nullptr) {}
      TransportMessage(const char* _path const char* _payload) : payload(_payload), path(_path) {}

      ~TransportMessage() {
        if (path) delete[] path;
        if (payload) delete[] payload;
      }

      const char* getPath() const { return path; }
      const char* getPayload() const { return payload; }

    private: 
      char* payload = nullptr;
      char* path = nullptr; 
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

namespace smartdevices::transport::mqtt {

    class MqttMessage final : public TransportMessage {

    };

    class MqttTransport final : public Transport {
    
    };
}

#endif