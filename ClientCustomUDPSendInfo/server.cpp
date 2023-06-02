
#include "clientServer.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

// Create Acknowledgement Packet
AckPacket createACKPacket(DataPacket dp) {
	AckPacket ackPacket;
	ackPacket.startOfPacketId = dp.startOfPacketId;
	ackPacket.clientId = dp.clientId;
	ackPacket.packetType = ACK_TYPE_PACKET;
    ackPacket.receivedSegmentNumber = dp.segmentNumber;
	ackPacket.endOfPacketId = dp.endOfPacketId;
	return ackPacket;
}

// Creating Reject Packet
RejectPacket createRejectPacket(DataPacket dp) {
	RejectPacket rejectPacket;
	rejectPacket.startOfPacketId = dp.startOfPacketId;
	rejectPacket.clientId = dp.clientId;
    rejectPacket.packetType = REJECT_TYPE_PACKET;
	rejectPacket.receivedSegmentNumber = dp.segmentNumber;
	rejectPacket.endOfPacketId = dp.endOfPacketId;
	return rejectPacket;
}

int main(){

    /// Storing segment number to identify duplicate packets
	int segmentBuffer[20];
	for (int j = 0; j < 20; j++) {
		segmentBuffer[j] = 0;
	}

   /// Create UDP Socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    /// Assign IP and Port for Server
    sockaddr_in serverAddress;
    socklen_t serverAddressSize;

	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(PORT);
	serverAddressSize = sizeof(serverAddress);

    if (bind(sock, reinterpret_cast<struct sockaddr*>(&serverAddress), serverAddressSize) != 0) {
        std::cout << "Error binding socket" << std::endl;
        return 1;
    }

    uint8_t expectedSegmentNumber = 1;
    bool lastPacketReceived = false;
    while(true && !lastPacketReceived){
        std::cout << "\n************************* Server is waiting for new Packet **************************" <<  std::endl;
        /// Wait for data packet
        DataPacket packet{};
        socklen_t addrLen = sizeof(serverAddress);
        ssize_t bytesRead = recvfrom(sock, reinterpret_cast<char*>(&packet), sizeof(DataPacket), 0,
                                     reinterpret_cast<sockaddr*>(&serverAddress), &addrLen);

        std::cout << "------- Server is Receiving New Paket : " << static_cast<int>(expectedSegmentNumber) << " -------" << std::endl;
        printDataPacket(packet);


        if(bytesRead > 0){
            if(packet.packetType == DATA_TYPE_PACKET){
                int len = strlen(packet.payload);
                /// Process Data Packet
                if(packet.segmentNumber == 11) { 
                    std::cout << "\nSERVER ASLEEP !" << std::endl;
		 		    sleep(15); /// To simulate that server is not responding, client will resend the packet
                    lastPacketReceived = true;
		     	}
                else if(len != packet.length){
                    RejectPacket rejectPacket = createRejectPacket(packet);
                    rejectPacket.rejectSubCode = LENGTH_MISMATCH; /// Length of the payload is not matching, packet rejected
                    std::cout << "\nLENGTH MISMATCHED - Packet Rejected" << std::endl;
                    sendto(sock, reinterpret_cast<char*>(&rejectPacket), sizeof(RejectPacket), 0, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
                    expectedSegmentNumber++;
                }
                else if (packet.endOfPacketId != END_PACKET_ID){
                    RejectPacket rejectPacket = createRejectPacket(packet);
                    rejectPacket.rejectSubCode = END_OF_PACKET_MISSING; /// End of Packet is missing, packet rejected
                    std::cout << "\nEND OF PACKET IS MISSING - Packet Rejected" << std::endl;
                    sendto(sock, reinterpret_cast<char*>(&rejectPacket), sizeof(RejectPacket), 0, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
                    expectedSegmentNumber++;
                }
                else if(packet.segmentNumber == expectedSegmentNumber){ 
                    AckPacket ackPacket = createACKPacket(packet); /// Valid Packet received, send ACK
                    std::cout << "\nVALID PACKET RECEIVED - Packet Acknowledged" << std::endl;
			        sendto(sock, reinterpret_cast<char*>(&ackPacket), sizeof(AckPacket), 0, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
                    expectedSegmentNumber++;
                }
                else if(packet.segmentNumber < expectedSegmentNumber) {
                    RejectPacket rejectPacket = createRejectPacket(packet);
                    rejectPacket.rejectSubCode = DUPLICATE_PACKET; /// Previously sent packet received again, packet rejected
                    std::cout << "\nDUPLICATE PACKET RECEIVED - Packet Rejected" << std::endl;
                    sendto(sock, reinterpret_cast<char*>(&rejectPacket), sizeof(RejectPacket), 0, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
                    expectedSegmentNumber++;
                }
                else if (packet.segmentNumber != expectedSegmentNumber){
                    RejectPacket rejectPacket = createRejectPacket(packet);
                    rejectPacket.rejectSubCode = OUT_OF_SEQUENCE; /// Did not receive the packet in sequence, packet rejected
                    std::cout << "\nPACKET IS OUT OF SEQUENCE - Packet Rejected" << std::endl;
                    std::cout << "\nExpected Segment Number " << static_cast<int>(expectedSegmentNumber) << ", Received Segment Number "<< static_cast<int>(packet.segmentNumber) << std::endl;
                    sendto(sock, reinterpret_cast<char*>(&rejectPacket), sizeof(RejectPacket), 0, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
                    expectedSegmentNumber++;
                }
            }
        }
    }


    return 0;
}