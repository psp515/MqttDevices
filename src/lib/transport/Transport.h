#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <vector>

#include "Logger.h"
#include "Configuration.h"

using namespace smartdevices::logging;
using namespace smartdevices::configuration;

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

    using MessageCallback = void (*)(const TransportMessage&);

    struct TransportCallback {
      const char* path;
      MessageCallback callback;
    };

    class Transport {
    public:
      explicit Transport(Configuration& configuration, Logger& logger) : _configuration(configuration), _logger(logger) {};

      virtual bool start() = 0;
      virtual bool reconnect() = 0;

      virtual bool send(TransportMessage message) = 0;
      virtual bool loop() = 0;
      virtual bool observe(const char* path, MessageCallback callback) = 0;

    protected:
      Logger& _logger;
      Configuration& _configuration;
      std::vector<TransportCallback> _callbacks;
    };
}

#endif