#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <string>
#include <vector>

#include "Logger.h"
#include "Configuration.h"

using namespace smartdevices::logging;
using namespace smartdevices::configuration;

namespace smartdevices::transport {

  class TransportMessage {
  public:
    TransportMessage() = default;

    TransportMessage(const std::string& _path, const std::string& _payload) : path(_path), payload(_payload) {}

    const std::string& getPath() const noexcept {
        return path;
    }

    const std::string& getPayload() const noexcept {
        return payload;
    }

  protected:
    std::string path;
    std::string payload;
  };

  using MessageCallback = void (*)(const TransportMessage&);

  struct TransportCallback {
    std::string path;
    MessageCallback callback;
  };

  class Transport {
  public:
    explicit Transport(Configuration& configuration, Logger& logger) : _configuration(configuration), _logger(logger) {}
    virtual ~Transport() = default;

    virtual bool setup() = 0;
    virtual bool loop() = 0;
    virtual bool reconnect() = 0;

    virtual bool send(const TransportMessage& message) = 0;
    virtual bool observe(const std::string& path, MessageCallback callback) = 0;

  protected:
      Logger& _logger;
      Configuration& _configuration;
      std::vector<TransportCallback> _callbacks;
  };

}

#endif