Instructions: https://docs.google.com/document/d/1SFsELva0n1u7v4M4CsRrHYBxsgNAHZbi-M4TcC9Eo5o/edit?usp=sharing

Keywords  
TCP/IP, sockets, packet capture, network security, sniffing, spoofing, social engineering  

Introduction  
In this programming assignment, you will attack a server using spoofing and sniffing to get valuable information back from the server. You need PCAP for this packet manipulation. Here is the PCP documentation.
Download and run the lab environment  
Clone the assignment, cd into the assignment folder
Go to the environment directory 
Switch to the seed user by typing sudo su seed
Run dcbuild
Run dcup to start the dockers
Get another shell and again switch to seed user
Run dockps to list the dockers running 
Run docksh <first two digits of id> to ssh into the corresponding docker
Inside the docker environment, cd to volumes, and you should see your PA code in there (you can still modify things as a normal user, no need to login every time)
Now use the victim container to host the server container attacker for the spoofer and sniffer. You need to open 2-3 terminals for this assignment, one docksh into the attacker and one or two docksh into the victim (read the implementation note for more info). Since we have the shared volumes folder, compiling code in one container directly affects the other.
Docker commands cheat sheet
These aliases are already defined for you in the .bash_aliases file of the seed user
Command
Alias
docker-compose build
dcbuild
docker-compose up
dcup 
docker-compose down
dcdown
docker ps --format "{{.ID}}  {{.Names}}"
dockps 
docker exec -it <id> /bin/bash
docksh  <id>


Requirements
In this assignment, you will write a sniffer and a spoofer program. The goal is to spoof packets with a fake identity accepted by the server. The server then parses the data from your packet to its reply message destination address along with the requested data.
Packet sniffing is the practice of intercepting and inspecting packets while they are flowing through the network. This can be for monitoring or malicious purposes.
Packet / IP spoofing can create Internet Protocol (IP) packets with a source IP address to conceal the sender's identity or impersonate another user.  
Social engineering is a deception technique to trick individual(s) into divulging confidential or personal information that may be used for fraudulent purposes. Figure 1 shows an IP spoofing attack in which the attacker gets confidential data from the server.
From some leaked documents, you learn that the server only processes packets from a specific address, 154.123.122.111, and drops everything else it receives. 
The security maintainer is an Aggie who did not take CSCE 313 seriously and forgot to put in any protection, so the server does not do any other checks besides looking at the sender’s IP address before processing the data. This means that by sending a request with the matching IP address, you can tell the server to execute various tasks, including sending back sensitive information. 

Figure 1: Basic IP spoofing attack. (All IP addresses above are for demonstration purposes. Read the document for the correct addresses)
Your goal is to implement the attack:
Write a spoofer that sends spoofed packets with the IP address 192.168.0.1 to the server with the IP address 10.9.0.5.
Write a sniffer with IP address 10.9.0.1 that monitors the network for confidential packets sent back from the victim.
Implementation Note
The server is already implemented (besides send_raw_ip_packet, which the server relies on). The only item that should be running on the victim container is the server; running it on the attacker will not work since the IP address is hard coded. All the correct IP addresses are provided to you in common.h, no need to modify those.
You will need to modify: sniff.cpp, spoof.cpp, and send_raw_ip_packet in common.h (no need to modify anything outside the TO DO comment)
Recommended completing steps: 
sniff.cpp -> spoof.cpp -> send_raw_ip_packet
for sniffer’s pcap_open_live, to get the correct interface, do ifconfig and select the one on top (for the pic below, it is br-5738f31b4abe as the correct interface)

you will need 3 shells to test: 1 for the victim, 2 for the attackers
boot up the server in victim
run sniffer in one shell then spoofer on the other shell
you can also run spoofer in the background and then sniffer if you want to put them in the same shell
spoofer and sniffer running correctly - note that we have multiple flags, so this might not be the one you will get, but all flags start with CSCE313｜

server running correctly

A working spoofer should be able to sniff any packet, and you can test using ping.
Look at the given PCAP lectures for additional hints
Getting started
Go to the assignment’s GitHub classroom: https://classroom.github.com/a/VrICNsh_
Create a repository for your project. 
Once completed and committed, you must submit the assignment on Gradescope:
The sniffer program sniff.cpp
The spoofer program spoof.cpp
common.h for the send_raw_ip_packet
A screenshot of your output showing the captured flag
How to run PA5 tutorial: https://www.youtube.com/watch?v=hrjIRgPmrqo 
