#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "Logger.h"
#include "Configuration.h"

namespace smartdevices::transport {

    class TransportMessage {
    public:
      TransportMessage() : payload(nullptr), path(nullptr) {}
      TransportMessage(const char* _path, const char* _payload) : payload(_payload), path(_path) {}

      ~TransportMessage() {
        if (path) delete[] path;
        if (payload) delete[] payload;
      }

      const char* getPath() const { return path; }
      const char* getPayload() const { return payload; }

    private: 
      const char* payload = nullptr;
      const char* path = nullptr; 
    };

    class Transport {
    public:

      explicit Transport(Configuration configuration, Logger& logger) : _configuration(configuration), _logger(logger) {};

      virtual bool start() = 0;
      virtual bool reconnect() = 0;

      virtual bool send(TransportMessage message) = 0;
      virtual void receive() = 0;
      virtual void observe(const char* path, MessageCallback callback) = 0;

      typedef void (*MessageCallback)(const TransportMessage&);

    protected:
      Logger _logger
      Configuration _configuration;
      std::vector<TransportCallback> _callbacks;

      struct TransportCallback {
        const char* path;
        MessageCallback callback;
      };
    };
}

#endif