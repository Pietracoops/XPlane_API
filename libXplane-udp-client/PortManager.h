#pragma once
#pragma once


#ifndef PORTMANAGER_SRC_PORTMANAGER_H_
#define PORTMANAGER_SRC_PORTMANAGER_H_


#include <future>
#include <iostream>
#include <string>
#include <vector>
#include <map>


struct ClientObj
{
	std::string name = "";
	unsigned int publishing_port = 0;
	unsigned int subscribing_port = 0;
};

class PortManager
{
private:

	int m_starting_port;						// Starting port in which to store and search for ports
	int m_max_clients;							// Max number of clients to consider
	std::vector<ClientObj> m_clients;			// Clients to be managing
	std::map<int, int> m_map_taken_ports;		// Store ports

	int returnAvailablePort();					// Return a port that is available
	void occupyPorts(int port1, int port2);		// Label the ports specified as occupied
	void liberatePorts(int port1, int port2);	// Label the ports specified as unoccupied

public:
	PortManager(int starting_port, int max_number_clients);
	~PortManager();

	
	std::pair<int, int> storeClient(std::string client_name);
	bool removeClient(std::string client_name);
	ClientObj getClient(std::string client_name);
	bool isConnected(std::string client_name);


};
#endif /* CLIENTMANAGER_SRC_CLIENTMANAGER_H_ */