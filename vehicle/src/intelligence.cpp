#include "intelligence.hpp"
#include "vehicle.hpp"

Intelligence::Intelligence(Vehicle *_vehicle) : vehicle(_vehicle)
{
    has_feedback = false;
}

void Intelligence::mainLoop()
{
    while(true)
    {
        if (hasCommand())
        {
            decodeMessage();
            sendFeedback();
        }
    }
    
}

bool Intelligence::hasCommand()
{
    return vehicle->communication.receiveFromRadio();
}

void Intelligence::followTheRoad()
{
}

void Intelligence::avoidObstacle()
{
}

void Intelligence::decodeMessage()
{
    bool has_feedback = false;
    uint8_t other_sensors_reading;
    float ultrassound_reading;
    MovementInfo movement;
    status::Status status;
    received_message = vehicle->communication.getData();
    switch (received_message.command)
    {
        case FORCE_GO_AHEAD:
            followTheRoad();
            break;
        case OPEN_BOX:
            vehicle->box.unlock();
            break;
        case CLOSE_BOX:
            vehicle->box.lock();
            break;
        case GET_QR_CODE:
            qr_codes_read.push_back(1);//vehicle->vision.getQrCodeFromUser())
            has_feedback = true;
        default:
            break;
    }
    switch (received_message.sensor_to_read)
    {
        case ULTRASOUND:
            ultrassound_reading = 0.2;//vehicle->vision.getUltrasoundReading();
            other_sensors_reading = NO_SENSOR_READ;
            has_feedback = true;
            break;
        case ITEM_DETECTOR:
            ultrassound_reading = 0;
            other_sensors_reading = uint8_t(vehicle->box.hasItem());
            has_feedback = true;
        case LOCK:
            ultrassound_reading = 0;
            other_sensors_reading = uint8_t(vehicle->box.isBoxClose());
            has_feedback = true;
        default:
            ultrassound_reading = 0;
            other_sensors_reading = NO_SENSOR_READ;
            break;
    }
    switch (received_message.required_status)
    {
        case MOVEMENT:
            movement = current_movement;
            movement.read = true;
            status = status::NO_STATUS;
            has_feedback = true;
            break;
        case VEHICLE_STATUS:
            movement.read = false;
            status = current_status;
            has_feedback = true;
        default:
            movement.read = false;
            status = status::NO_STATUS;
            break;
    }
    path_to_follow = received_message.path;
    target_qr_code = received_message.qr_code;
    if(has_feedback)
    {
        sent_message = SentMessage(qr_codes_read, ultrassound_reading, other_sensors_reading, movement, status);
    }
}

void Intelligence::sendFeedback()
{
    vehicle->communication.sendToRadio(sent_message);
}

void Intelligence::waitTrafficLight()
{    
}

int Intelligence::getQrCode()
{
    return 0;
}
