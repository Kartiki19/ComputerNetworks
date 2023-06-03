#ifndef CLIENTSERVER_H
#define CLIENTSERVER_H
#include <cstdint> 
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>

/// Protocol Premitives
#define START_PACKET_ID 0xFFFF
#define END_PACKET_ID 0xFFFF
#define CLIENT_ID 0xFF

#define LENGTH_MAX 0xFF
#define TIMER 3
#define MAX_ITERATIONS 3
#define PORT 8080

/// Identification Types
enum Status {
    ACC_PERMISSION = 0xFFF8,
    NOT_PAID = 0xFFF9,
    NOT_EXIST = 0xFFFA,
    ACCESS_OK = 0xFFFB,
    TECHNOLOGY_MISMATCH = 0xFFFC
};

/// Structure of a Packet
struct PacketFormat {
	uint16_t startOfPacketId;
	uint8_t clientId;
	uint16_t type;
	uint8_t segmentNumber;
	uint8_t length;
	uint8_t technology;
	unsigned int sourceSubscriberNum;
	uint16_t endOfPacketId;
};

void printPacket(PacketFormat packet){
    std::cout << "Start ID: " << packet.startOfPacketId << std::endl;
    std::cout << "Client ID: " << static_cast<int>(packet.clientId) << std::endl;
    std::cout << "Type: " << static_cast<Status>(packet.type) << std::endl;
    std::cout << "Segment Number: " << static_cast<int>(packet.segmentNumber) << std::endl;
    std::cout << "Length: " << static_cast<int>(packet.length) << std::endl;
    std::cout << "Technology: " << static_cast<int>(packet.technology) << std::endl;
    std::cout << "Source Subscriber No.: " << packet.sourceSubscriberNum << std::endl;
    std::cout << "End ID: " << packet.endOfPacketId << std::endl;
}

#endif