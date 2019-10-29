#ifndef message_hpp_
#define message_hpp_

#include <vector>

struct MovementInfo{
    bool read;
    float amplitude;
    float curvature;
};

enum Commands{
    NO_COMMAND = 0,
    FORCE_GO_AHEAD,
    OPEN_BOX,
    CLOSE_BOX,
    GET_QR_CODE
};

enum PossibleSensors{
    NO_SENSORS = 0,
    ITEM_DETECTOR,
    ULTRASOUND,
    LOCK
};

enum PossibleStatus{
    NO_STATUS = 0,
    MOVEMENT,
    VEHICLE_STATUS
};

namespace status {
    enum Status{
        NO_STATUS = 0,
        STOPPED, 
        MOVING_FORWARD, 
        MOVING_BACKWAY, 
        AVOIDING_OBSTACLE, 
        WAITING_SEMAPHORE, 
        WAITING_ROUTE
    };
}

class ReceivedMessage{
    public:
        std::vector<uint8_t> path;
        uint16_t qr_code;
        Commands command; 
        PossibleSensors sensor_to_read;
        PossibleStatus required_status;
        ReceivedMessage(std::vector<uint8_t> _path, uint16_t _qr_code, Commands _command, PossibleSensors _sensor_to_read, PossibleStatus _required_status):
            path(_path), qr_code(_qr_code), command(_command), sensor_to_read(_sensor_to_read), required_status(_required_status) {}
};

class SentMessage{
    public:
        std::vector<uint16_t> qr_codes_read;
        float ultrassound_reading;
        uint8_t other_sensors_reading;
        MovementInfo movement;
        status::Status status;
        SendedMessage(std::vector<uint16_t> _qr_codes_read, float _ultrassound_reading, bool _other_sensors_reading, MovementInfo _movement, status::Status _status):
            qr_codes_read(_qr_codes_read), ultrassound_reading(_ultrassound_reading), other_sensors_reading(_other_sensors_reading), movement(_movement), status(_status) {}
        SendedMessage(){;}
};

#define NO_SENSOR_READ 3

#endif
