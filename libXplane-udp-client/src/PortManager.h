#pragma once

#ifndef PORTMANAGER_SRC_PORTMANAGER_H_
#define PORTMANAGER_SRC_PORTMANAGER_H_


#include <string>
#include <unordered_map>

class PortManager
{
private:

	int m_StartingPort;											// Starting port in which to store and search for ports
	int m_MaximumNumberOfClients;								// Max number of clients to consider
	std::unordered_map<unsigned int, bool> m_OccupiedPorts;		// Store ports

public:
	PortManager(unsigned int starting_port, unsigned int max_number_clients);
	~PortManager();
	
	int returnAvailablePort();
	void occupyPort(unsigned int port);
	void liberatePort(unsigned int port);
};
#endif /* CLIENTMANAGER_SRC_CLIENTMANAGER_H_ */