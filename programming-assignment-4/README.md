Keywords  
TCP/IP, sockets, Multithreading, producer-consumer relationship, race conditions, overflow, underflow, mutual exclusion.  
  
Introduction  
In this programming assignment, you add a class called TCPRequestChannel to extend the IPC capabilities of the client-server implementation in PA3 using the TCP/IP protocol. The client-side and server-side ends of a TCPRequestChannel will reside on different computers.
Since the communication API (not just the underlying functionality and features) through TCP/IP is different from FIFO, you will also need to restructure the server.cpp and client.cpp as part of this programming assignment:
The server program must be modified to handle incoming requests across the network request channels using the TCP/IP protocol. The server must be able to handle multiple request channels from the client residing on a different computer.
You must also modify the client to send requests across the network request channels using the TCP/IP protocol.  
  
Requirements  
In this assignment, you will restructure your PA3 client.cpp in order to accomplish data point and file transfers using TCP/IP request channels. You will also rewrite your server.cpp so that multiple instances of the client program can connect to the server simultaneously. 
The client will take in two additional mandatory arguments compared to PA3. 
“-a” which denotes the IP address the server is running on.
“-r” which denotes the port number the server is deploying its services to.
The following command line options are valid for PA4:
For data point transfers: a, r    [with optional n, w, b, p, h, m arguments]
./client -n <# data items> -w <# workers> -b <BoundedBuffer size> -p <# patients> -h <# histogram threads> -m <buffer capacity> -a <IP address> -r <port no>

For file transfers: a, r, f   [with optional w, b, m arguments]
./client -w <# workers> -b <BoundedBuffer size> -m <buffer capacity> -f <filename> -a <IP address> -r <port no>

The server must be called in the following form:  
./server -r <port no> -m <buffer capacity>

“-m” is an optional argument whereas “-r” is a mandatory argument for the server.
The server no longer runs using the exec() function from the client. It is run separately from a terminal on a different computer. 
The server executable does not need the ip address argument because it runs the service in the current host. It just needs to deploy its services on the user-assigned port.
The client, on the other hand, needs to know where the server is running (i.e., IP address) and at which port number.
Implementation Note
A TCPRequestChannel class was added to replace the FIFORequestChannel class from PA3. The following matches the class declaration (comments explaining functions in starter code):
class TCPRequestChannel {
private:
	int sockfd;

public:
	TCPRequestChannel (const std::string _ip_address,
				 const std::string _port_no);
	TCPRequestChannel (int _sockfd);

	~TCPRequestChannel ();

	int accept_conn ();

	int cread (void* msgbuf, int msgsize);
	int cwrite (void* msgbuf, int msgsize);
};


You should provide definitions for the functions declared above in a TCPRequestChannel.cpp file. Also, you will need to make changes in the server.cpp and the client.cpp programs to implement the desired functionality for realizing TCP/IP connection. For instance, the server should run an infinite loop to accept() incoming connections from the client and create a new thread with the accepted socket to process subsequent communication using the handle_process_loop function (which should stay mostly unchanged except for the input argument type - it should now be TCPRequestChannel* instead of FIFORequestChannel*).
The client on the other hand, can simply call the respective constructor  w times to create all the channels. There is no need for sending a NEWCHANNEL_MSG to the server because calling the constructor results in a separate dedicated channel. Due to this reason, the client does not need a separate control channel either. 
Create TCPRequestChannel.h/.cpp from the template code above.
Fulfill requirements of network socket connections in TCPRequestChannel
Modify client.cpp/server.cpp
Server creates TCPRequestChannel(ip_address=“”, port_no=r)
Server enters an infinite loop to establish connections with TCPRequestChannel::accept_conn() and TCPRequestChannel::TCPRequestChannel(int sockfd)
Client creates channels with TCPRequestChannel(ip_address=a, port_no=r)
