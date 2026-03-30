#ifndef CLOCK_SERVICE_H
#define CLOCK_SERVICE_H

namespace smartdevices::clock {

    class ClockService
    {
    public:
        virtual ~ClockService() = default;

        virtual bool setup() = 0;
        virtual bool loop() = 0;
    };

}

#endif