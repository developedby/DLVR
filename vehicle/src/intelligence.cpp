#include "intelligence.hpp"
#include "vehicle.hpp"
#include <iostream>
#include <vector>
#include "geometry.hpp"
#include "message.hpp"

Intelligence::Intelligence(Vehicle *_vehicle):
    vehicle(_vehicle), street_follower(_vehicle, std::vector<uint8_t>(), 0)
{
    this->has_feedback = false;
    this->is_traffic_light_red = false;
}

void Intelligence::mainLoop()
{
    std::cout << "main loop init" << std::endl;
    this->current_status = status::WAITING_ROUTE;
    while(vehicle->communication.isChipConnected())
    {
        // Checa comando novo
        if ((this->vehicle->communication).has_new_msg)
        {
            // Decodifica e age se necessario
            this->decodeMessage();
            // Manda update pro server
            this->sendFeedback();
        }
        // Ciclo de movimento
        if (this->street_follower.hasRoute())
        {
            //if (not this->is_traffic_light_red)
            {
                this->current_status = this->street_follower.current_status;
                this->street_follower.followPath();
            }
            /*else
            {
                std::cout << "semafaro vermelho" << std::endl;
                this->current_status = status::WAITING_SEMAPHORE;
            }*/
        }
        // Checa markers
        this->processForwardImg();
    }
    std::cout << "radio desconectado" << std::endl;    
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
    status::Status status_;
    this->received_message = this->vehicle->communication.getData();
    std::cout << "mensagem: " << std::endl;
    std::cout << "comando: " << this->received_message.command << " sensor para leitura: " << this->received_message.sensor_to_read << " status: " << this->received_message.required_status << std::endl;
    switch (this->received_message.command)
    {
        case FORCE_GO_AHEAD:
            this->street_follower.followPath();
            break;
        case OPEN_BOX:
            this->vehicle->box.unlock();
            break;
        case CLOSE_BOX:
            this->vehicle->box.lock();
            break;
        case GET_QR_CODE:
            has_feedback = true;
        default:
            break;
    }
    switch (this->received_message.sensor_to_read)
    {
        case ULTRASOUND:
            ultrassound_reading = vehicle->vision.distanceFromObstacle();
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
    switch (this->received_message.required_status)
    {
        case MOVEMENT:
            movement = current_movement;
            movement.read = true;
            status_ = status::NO_STATUS;
            has_feedback = true;
            break;
        case VEHICLE_STATUS:
            movement.read = false;
            status_ = this->current_status;
            std::cout << "status eh: " << status_ << std::endl;
            has_feedback = true;
            break;
        default:
            movement.read = false;
            status_ = status::NO_STATUS;
            break;
    }

    std::cout << "caminho recebido: " << std::endl;
    for (const auto step: received_message.path)
    {
        std::cout << int(step) << " ";
    }
    std::cout << std::endl << "qr code: " << received_message.qr_code << std::endl;

    if(has_feedback)
    {
        std::cout << "tem feedback" << std::endl;
        this->sent_message = SentMessage(this->qr_codes_read, ultrassound_reading, other_sensors_reading, movement, status_);
        std::cout << " ultrassom: " <<  this->sent_message.ultrassound_reading << " others: " << this->sent_message.other_sensors_reading << " status: " << this->sent_message.status_ << std::endl;
        std::cout << "status=" << status_ << " sent_message.status=" << this->sent_message.status_ << std::endl;
        this->qr_codes_read = std::vector<uint16_t>();
    }
    
    if(not received_message.path.empty())
    {
        this->street_follower = StreetFollower(this->vehicle, received_message.path, received_message.qr_code);
    }
    else
    {
        this->target_user_qr_code = received_message.qr_code;
        this->searchUserQRCode();
    }
    this->vehicle->communication.has_new_msg = false;
}

void Intelligence::sendFeedback()
{
    std::cout << "mandando feedback" << std::endl;
    this->vehicle->communication.sendToRadio(this->sent_message);
}

void Intelligence::processForwardImg()
{
    this->vehicle->vision.getForwardCamImg();
    this->is_traffic_light_red = vehicle->vision.isTrafficLightRed();
    auto [ids, positions] = vehicle->vision.findCityARMarkers();
    if(this->qr_codes_read.size() < ids.size())
    {
        for(auto id:ids)
        {
            std::cout << "qr encontrado: " << id << std::endl;
            this->qr_codes_read.push_back(id);
        }
    }
    else
    {
        std::vector<uint16_t> aux;
        for(int i=0; i < int(ids.size()); i++)
        {
            for(auto id:ids)
            {
                if (id != qr_codes_read[int(qr_codes_read.size()) -1 + i])
                { 
                    std::cout << "qr encontrado: " << id << std::endl;
                    aux.push_back(id);
                }
            }
        }
        for(auto id:aux)
        {
            this->qr_codes_read.push_back(id);
        }
    }
}

void Intelligence::searchUserQRCode()
{
    this->vehicle->vision.getForwardCamImg();
    auto [ids, corners] = this->vehicle->vision.findAppARMarkers();
    for (auto id: ids)
        if (id == this->target_user_qr_code)
            this->vehicle->box.unlock();
}
