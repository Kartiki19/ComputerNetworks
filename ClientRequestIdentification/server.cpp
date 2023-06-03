#include "clientServer.h"
#include <map>

#define SUBSCRIBER_COUNT 10

/// Verification Database for Subscribers
struct SubscriberDatabase {
	unsigned int subscriberNumber;
	uint8_t technology;
	int status; /// Paid, Not_Paid, Not_Found
};

/// To read the database file and create subscriber database structures
void readDatabase (std::map<unsigned int, SubscriberDatabase>& subscriberDB) {
	char line[30];
	FILE *verificationDBPtr;
	verificationDBPtr = fopen("Verification_Database.txt", "rt");

    if(verificationDBPtr == NULL) {
        std::cerr << "Failed to Open the File input.txt" << std::endl;
        exit(0);
    }

	while (fgets(line, sizeof(line), verificationDBPtr) != NULL)
	{
        SubscriberDatabase subscriber;
        char* sourceSubscriberNum = strtok(line," ");
        subscriber.subscriberNumber = static_cast<unsigned int>(atoi(sourceSubscriberNum));

        char* technology = strtok(NULL," "); /// NULL for continuing the same string
        subscriber.technology = atoi(technology);

        char* status = strtok(NULL," "); /// NULL for continuing the same string
        subscriber.status = atoi(status);

        subscriberDB[subscriber.subscriberNumber] = subscriber;
	}

	fclose(verificationDBPtr);
}

/// @brief Verify Subscriber exist or not, if does, check if paid or not and if technology matchesand update responce packet
/// @param subscriberDB : Database for all the existing subscribers with their status
/// @param packet : Received Request packet to verify
void verifySubscriber(std::map<unsigned int, SubscriberDatabase> &subscriberDB, PacketFormat& packet) {
    unsigned int subscriberNumber = packet.sourceSubscriberNum;
    if (subscriberDB.find(subscriberNumber) != subscriberDB.end())
    {
        if (subscriberDB[subscriberNumber].technology == packet.technology){
            if (subscriberDB[subscriberNumber].status == 1){
                std::cout << "\n••••• Subscriber has paid, permitted access to the network ! ! •••••" << std:: endl;
                packet.type = Status::ACCESS_OK;
            }
            else{
                std::cout << "\n••••• Subscriber has not Paid ! •••••" << std:: endl;
                packet.type = Status::NOT_PAID;
            }
        }
        else {
            std::cout << "\n••••• Technology of Subscriber did not Matched ! •••••" << std:: endl;
            packet.type = Status::TECHNOLOGY_MISMATCH;
        }
    }
    else{ 
        std::cout << "\n••••• Subscriber Does Not Exist ! •••••" << std:: endl;
        packet.type = Status::NOT_EXIST;
    }
}

int main(){
    /// Read the Verification Database and create subscriber request database
    std::map<unsigned int, SubscriberDatabase> subscribers;
    readDatabase(subscribers);

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

    bool lastPacketReceived = false;
    while(!lastPacketReceived){
        std::cout << "\n************************* Server is waiting for new Packet **************************" <<  std::endl;
        /// Wait for data packet
        PacketFormat packet{};
        ssize_t bytesRead = recvfrom(sock, reinterpret_cast<char*>(&packet), sizeof(PacketFormat), 0,
                                     reinterpret_cast<sockaddr*>(&serverAddress), &serverAddressSize);

        std::cout << "••••• Server is Receiving New Paket : " << static_cast<int>(packet.segmentNumber) << " •••••" << std::endl;

        if(bytesRead > 0 && packet.type == Status::ACC_PERMISSION){
            /// Simulate that server is asleep for the last packet
            if(packet.segmentNumber == 10) {
                std::cout << "\nSERVER ASLEEP !" << std::endl;
                sleep(15); /// To simulate that server is not responding, client will resend the packet
                lastPacketReceived = true;
                return 0;
            }

            printPacket(packet);

            /// Verify the subscriber
            verifySubscriber(subscribers, packet);
            
            /// Send verification report back to client
            sendto(sock, reinterpret_cast<char*>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
                   
		}
    }
    return 0;
}