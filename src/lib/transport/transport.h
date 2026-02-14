#ifndef MESSAGING_SERVICE_H
#define MESSAGING_SERVICE_H



namespace mqttdevices::transport {

    class Message {
    public:
            

    private: 
        char* payload;
        char* id;
        
    };

    class TransportService {
    public:
        virtual void start();
        virtual bool send(Message message);
        virtual void receive();
        virtual void observe(const char* id, const char* payload);
    };
}

#endif