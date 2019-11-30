#ifndef intelligence_hpp_
#define intelligence_hpp_

#include <vector>
#include "message.hpp"
#include "street_follower.hpp"

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
        StreetFollower street_follower;
        bool has_feedback;
        bool is_traffic_light_red;
        uint16_t target_user_qr_code;

    public:
        Intelligence (Vehicle* _vehicle);
        void mainLoop();
        bool hasCommand();
        void followThePath();
        void followTheRoad(float distance_to_go);
        void avoidObstacle();
        void decodeMessage();
        void sendFeedback();
        void getQrCodeFromCity();
        void getQrCodeFromUser();
        void processForwardImg();
        void goToCityQrCode();
        void searchUserQRCode();
};
#endif
