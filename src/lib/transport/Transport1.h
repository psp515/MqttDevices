#ifndef TRANSPORT_H
#define TRANSPORT_H
#define CALLBACK_SIGNATURE void (*callback)(char*, uint8_t*, unsigned int)

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
        typedef void (*MessageCallback)(const TransportMessage&);

        virtual bool start() = 0;
        virtual bool send(TransportMessage message) = 0;
        virtual void receive() = 0;
        virtual void observe(const char* id, MessageCallback callback) = 0;
    
    };
}

#endif