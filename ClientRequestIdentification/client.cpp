#include "clientServer.h"

using namespace std;

PacketFormat createPacket(){
    PacketFormat packet;
    packet.startOfPacketId = 0xFFFF;
    packet.clientId = 0xFF;
    packet.type = Status::ACC_PERMISSION;
    packet.endOfPacketId = 0xFFFF;
    return packet;
}

int main(){

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

    /// Creating the Packet
    PacketFormat requestPacket = createPacket();

    /// Reading Input
    FILE *inputFile = fopen("input.txt", "rt");
    if(inputFile == NULL) {
        cerr << "Failed to Open the File input.txt" << endl;
        return 1;
    }

    char line[30]; /// Characters in 1 line of input file ( Subscriber Number + technology)
    int segmentNum = 1; /// To keep track of segments
    while(fgets(line, sizeof(line), inputFile)){
        cout << "\n******************* New Request : "<< segmentNum <<" *******************\n";
        char* sourceSubscriberNum = strtok(line," ");
        requestPacket.length = strlen(sourceSubscriberNum);
        requestPacket.sourceSubscriberNum = atoi(sourceSubscriberNum);

        char* technology = strtok(NULL," "); /// NULL for continuing the same string
        requestPacket.length += strlen(technology);
        requestPacket.technology = atoi(technology);
        
        requestPacket.segmentNumber = segmentNum;
        printPacket(requestPacket);

        int iteration = 0;
        bool sentSuccessfully = false;
        while(iteration < MAX_ITERATIONS && !sentSuccessfully){
            /// Sending Request Packet to the Server
            sendto(sock, &requestPacket,sizeof(PacketFormat), 0, (struct sockaddr *)&serverAddress, serverAddressSize);
            iteration++;

			/// Receiving the response from server, receivedBytes will be less than 0, if there is a timeout (3 sec timer)
            PacketFormat responsePacket;
			int received = recvfrom(sock, &responsePacket, sizeof(PacketFormat), 0, NULL, NULL);
			if(received <= 0){
                cout << "\n••••• ACK not received •••••" << endl;
                sentSuccessfully = false;
            }
            else {
                if(responsePacket.type == Status::NOT_PAID){
                    cout << "\n••••• Subscriber has not paid ! •••••" << endl;
                }
                else if (responsePacket.type == Status::NOT_EXIST){
                    cout << "\n••••• Subscriber Does Not Exist ! •••••" << endl;
                }
                else if (responsePacket.type == Status::ACCESS_OK){
                    cout << "\n••••• Subscriber has paid, permitted access to the network ! •••••" << endl;
                }
                else if(responsePacket.type == Status::TECHNOLOGY_MISMATCH){
                     cout << "\n••••• Technology of Subscriber did not Matched ! •••••" << endl;
                }
                sentSuccessfully = true;
            }
        }

        if(iteration >= MAX_ITERATIONS){ /// After re-sending the packet 3 times, the ACK is not received
            cout << "\n************** SERVER DID NOT RESPOND **************" << endl;
            exit(0);
        }

        segmentNum++;
    }

    fclose(inputFile);
    return 0;
}