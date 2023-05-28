#include "clientServer.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;

/// Create Data Packet
DataPacket createDataPacket(){
    DataPacket dataPacket;
	dataPacket.startOfPacketId = START_PACKET_ID;
	dataPacket.clientId = 0x01;
    dataPacket.packetType = DATA_TYPE_PACKET;
	dataPacket.endOfPacketId = END_PACKET_ID;
	return dataPacket;
}

int main()
{
    /// Create UDP Socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        cerr << "Failed to create socket" << endl;
        return 1;
    }

    /// Assign IP and Port for Server
    sockaddr_in serverAddress;
    socklen_t serverAddressSize;

	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port=htons(PORT);
	serverAddressSize = sizeof(serverAddress);

    /// Setting ACK Timer of 3 sec
    timeval timer;
    timer.tv_sec = TIMER;
    timer.tv_usec = 0;

    /// Setting options for socket to wait for 3 sec for receiving data
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer)) == -1) {
        cerr << "Failed to set receive timeout : " << strerror(errno) << endl;
        close(sock);
        return 1;
    }

    /// Getting data from text file
    FILE *fileptr = fopen("payload.txt", "rt");
    if(fileptr == NULL){
        cerr << "Failed to Open the File payload.txt" << endl;
        return 1;
    }

    /// Read the file
    char line[255]; /// max payload for 1 packet
    int segmentNumber = 1;
    while(fgets(line, sizeof(line), fileptr) != NULL)
    {    
        DataPacket dataPacket = createDataPacket();
        dataPacket.segmentNumber = static_cast<uint8_t>(segmentNumber);
        cout << "\n******************* Sending New Packet : " << segmentNumber << " *******************" << endl;
        strcpy(dataPacket.payload, line);
        dataPacket.length = static_cast<uint8_t>(strlen(dataPacket.payload));

        /// Each payload file line has different cases to simulate packets such as, valid packets, duplicate packets, etc.

        switch (segmentNumber)
        {
        case 7:
            dataPacket.segmentNumber = 2; /// duplicate packet, sending packet no. 2 again [reject sub code : DUPLICATE_PACKET]
            break;

        case 8:
            dataPacket.length++; /// Increase the length to simulate length mismatch [reject sub code : LENGTH_MISMATCH]
            break;

        case 9:
            dataPacket.endOfPacketId = 0; /// Missing End of packet indentifies [reject sub code : END_OF_PACKET_MISSING]
            break;

        case 10:
            dataPacket.segmentNumber = dataPacket.segmentNumber + 3; /// Not Sending the packet in squence [reject sub code : OUT_OF_SEQUENCE]
            break;
        }

        cout << "--------- CLIENT SENDING ---------" << endl;
        printDataPacket(dataPacket);

        int iteration = 0;
        bool sentSuccessfully = false;
        while(iteration < MAX_ITERATIONS && !sentSuccessfully){
            /// Sending the Packet to the server
            sendto(sock, &dataPacket, sizeof(DataPacket), 0, (sockaddr *)&serverAddress, serverAddressSize);
            iteration++;
            
            /// Receiving the response from server, receivedBytes will be less than 0, if there is a timeout
            RejectPacket receivedPacket;
            int received = recvfrom(sock, &receivedPacket, sizeof(struct RejectPacket), 0, NULL, NULL);

            if(received <= 0){
                cout << "\n********* ACK not received *********" << endl;
                sentSuccessfully = false;
            }
            else if(receivedPacket.packetType == ACK_TYPE_PACKET){
                cout << "\nACK Received for packet " << static_cast<int>(receivedPacket.receivedSegmentNumber) << endl;
                sentSuccessfully = true;
            }
            else if(receivedPacket.packetType == REJECT_TYPE_PACKET){
                cout << "\n--------- Packet " << static_cast<int>(receivedPacket.receivedSegmentNumber) << " Rejected ---------" << endl;
                cout << "The Reject Type is: " << receivedPacket.rejectSubCode << endl;

                switch(receivedPacket.rejectSubCode){
                    case OUT_OF_SEQUENCE:
                        cout << "The received packet " << static_cast<int>(receivedPacket.receivedSegmentNumber) << " is out of sequence " << endl;
                    break;

                    case LENGTH_MISMATCH:
                        cout << "The Length of received packet "<< static_cast<int>(receivedPacket.receivedSegmentNumber) << " is mismatched " << endl; 
                    break;

                    case END_OF_PACKET_MISSING:
                        cout << "End of Packet is missing for the packet " << static_cast<int>(receivedPacket.receivedSegmentNumber) << endl;
                    break;

                    case DUPLICATE_PACKET:
                        cout << "Duplicate Packet : Packet " << static_cast<int>(receivedPacket.receivedSegmentNumber) << " is sent again "<< endl;
                    break;
                }
                sentSuccessfully = true;
            }
        }

        if(iteration >= MAX_ITERATIONS){ /// After re-sending the packet 3 times, the ACK is not received
            cout << "\n************** SERVER DID NOT RESPOND **************" << endl;
            exit(0);
        }
        segmentNumber++;
    }

    return 0;
}