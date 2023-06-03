# ComputerNetworks
Projects based on Computer Network Technologies

# Problem Statement
1. Simulate communication between one client and one server. 
2. Client is using customized protocol on top of UDP protocol, for sending information to the server.
3. Data Packets should be sent sequntially.
4. After receiving data packets, server will verify them and categorize the as Valid Packet, Duplicate Packet, Not In Sequence Packet, End of Packet is Missing and Length Mismatch Packet.
5. Also, simulate that server is asleep and not sending acknowldge to client. 
6. Client will wait for acknowledgment for 3 sec and if not received re-try sending the packet for 3 times.

# Execution
1. To run the program use Makefile: "make"
2. To re-compile, first clean all the executable files using : "make clean"
3. Check the Output_Screenshots folder for the demo execution and outputs
