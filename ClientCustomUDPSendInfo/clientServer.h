#ifndef CLIENTSERVER_H
#define CLIENTSERVER_H

#include <cstdint> 
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

/// Protocol Premitives
#define START_PACKET_ID 0xFFFF
#define END_PACKET_ID 0xFFFF
#define CLIENT_ID 0xFF

#define LENGTH_MAX 0xFF
#define TIMER 3
#define MAX_ITERATIONS 3
#define PORT 8080

/// Packet Types
#define DATA_TYPE_PACKET 0xFFF1
#define ACK_TYPE_PACKET 0xFFF2
#define REJECT_TYPE_PACKET 0xFFF3

/// Reject Sub Codes
#define OUT_OF_SEQUENCE 0xFFF4
#define LENGTH_MISMATCH 0xFFF5
#define END_OF_PACKET_MISSING 0xFFF6
#define DUPLICATE_PACKET 0xFFF7

#define PAYLOAD_SIZE 255

/// Data packet structure
///#pragma pack(1)
struct DataPacket {
    uint16_t startOfPacketId;
    uint8_t clientId;
    uint16_t packetType;
    uint8_t segmentNumber;
    uint8_t length;
    char payload[PAYLOAD_SIZE];
    uint16_t endOfPacketId;
};

/// ACK packet structure
////#pragma pack(1)
struct AckPacket {
    uint16_t startOfPacketId;
    uint8_t clientId;
    uint16_t packetType;
    uint8_t receivedSegmentNumber;
    uint16_t endOfPacketId;
};

/// Reject packet structure
//#pragma pack(1)
struct RejectPacket {
    uint16_t startOfPacketId;
    uint8_t clientId;
    uint16_t packetType;
    uint8_t receivedSegmentNumber;
    uint16_t rejectSubCode;
    uint16_t endOfPacketId;
};

/// To print the data packet
void printDataPacket(DataPacket dataPacket) {
	std::cout << "START OF PACKET ID : " << dataPacket.startOfPacketId << std::endl;
	std::cout << "CLIENT ID : " << static_cast<int>(dataPacket.clientId) << std::endl;
    std::cout << "PACKET TYPE : ";
    
    if(dataPacket.packetType == DATA_TYPE_PACKET) std::cout << "Data Packet" << std::endl;
    else if(dataPacket.packetType == ACK_TYPE_PACKET) std::cout << "Ack Packet" << std::endl;
    else if(dataPacket.packetType == REJECT_TYPE_PACKET) std::cout << "Reject Packet" << std::endl;

	std::cout << "SEGMENT NUMBER : " << static_cast<int>(dataPacket.segmentNumber) << std::endl;
	std::cout << "LENGTH : "<< static_cast<int>(dataPacket.length) << std::endl;
	std::cout << "PAYLOAD : " << dataPacket.payload;
	std::cout << "END OF DATA PACKET ID : " << dataPacket.endOfPacketId << std::endl;
}


#endif