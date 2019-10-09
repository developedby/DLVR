#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <vector>
#include <string>

class ReceivedMessage{
    public:
        std::vector<uint8_t> path;
        std::string qr_code;
        commands command; 
        possible_sensors sensor_to_read;
        possible_status required_status;
        ReceivedMessage(std::vector<uint8_t> _path, std::string _qr_code, commands _command, possible_sensors _sensor_to_read, possible_status _required_status)
        {
            path = _path;
            qr_code = _qr_code;
            command = _command; 
            sensor_to_read = _sensor_to_read;
            required_status = _required_status;
        }
};

class SendedMessage{
    public:
        std::vector<std::string> qr_codes_read;
        float ultrassound_reading;
        uint8_t other_sensors_reading;
        movement movement;
        status status;
        SendedMessage(std::vector<std::string> _qr_codes_read, float _ultrassound_reading, bool _other_sensors_reading, movement _movement, status _status)
        {
            qr_codes_read = _qr_codes_read;
            ultrassound_reading = _ultrassound_reading;
            other_sensors_reading = _other_sensors_reading; 
            movement = _movement;
            status = _status;
        }
};

struct movement{
    bool read;
    float amplitude;
    float curvature;
};

enum commands{
    NO_COMMAND = 0,
    FORCE_GO_AHEAD,
    OPEN_BOX,
    CLOSE_BOX,
    GET_QR_CODE
};

enum possible_sensors{
    NO_SENSORS = 0,
    ITEM_DETECTOR,
    ULTASOUND,
    LOCK
};

enum possible_status{
    NO_STATUS = 0,
    MOVEMENT,
    VEHICLE_STATUS
};

enum status{
    NO_STATUS = 0,
    STOPPED, 
    MOVING_FORWARD, 
    MOVING_BACKWAY, 
    AVOIDING_OBSTACLE, 
    WAITING_SEMAPHORE, 
    WAITING_ROUTE
};

#define NO_SENSOR_READ 3

#endif