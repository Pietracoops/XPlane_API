#include "PortManager.h"

PortManager::PortManager(unsigned int starting_port, unsigned int max_number_clients)
{
	m_StartingPort = starting_port;
	m_MaximumNumberOfClients = max_number_clients;

	for (unsigned int port_number = starting_port; port_number < (starting_port + (max_number_clients * 2)); port_number++) m_OccupiedPorts[port_number] = false;
}

PortManager::~PortManager()
{

}

int PortManager::returnAvailablePort()
{
	for (auto const& [port_number, occupied] : m_OccupiedPorts) if (!occupied) return port_number;

	return -1;
}

void PortManager::occupyPort(unsigned int port)
{
	m_OccupiedPorts[port] = true;
}

void PortManager::liberatePort(unsigned int port)
{
	m_OccupiedPorts[port] = false;
}