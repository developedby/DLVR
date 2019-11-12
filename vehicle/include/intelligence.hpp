#ifndef intelligence_hpp_
#define intelligence_hpp_
#include "message.hpp"

class Vehicle;

class Intelligence
{
    private:
        Vehicle *vehicle;
        ReceivedMessage received_message;
        SentMessage sent_message;
        std::vector<uint16_t> qr_codes_read;
        status::Status current_status;
        MovementInfo current_movement;
        std::vector<uint8_t> path_to_follow;
        uint16_t target_qr_code;
        bool has_feedback;
    public:
        Intelligence (Vehicle* _vehicle);
        void mainLoop();
        bool hasCommand();
        void followTheRoad();
        void avoidObstacle();
        void decodeMessage();
        void sendFeedback();
        void waitTrafficLight();
        int getQrCode();
};
#endif
