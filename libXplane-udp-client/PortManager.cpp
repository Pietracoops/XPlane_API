
#include "PortManager.h"


void PortManager::liberatePorts(int port1, int port2)
{
	for (auto const& [key, val] : m_map_taken_ports)
	{
		if (key == port1 || key == port2)
		{
			m_map_taken_ports[key] = 0;
		}
	}

}

void PortManager::occupyPorts(int port1, int port2)
{
	for (auto const& [key, val] : m_map_taken_ports)
	{
		if (key == port1 || key == port2)
		{
			m_map_taken_ports[key] = 1;
		}
	}

}

int PortManager::returnAvailablePort()
{
	for (auto const& [key, val] : m_map_taken_ports)
	{
		if (val == 0)
		{
			return key;
		}
	}

	return -1;
}

bool PortManager::removeClient(std::string client_name)
{

	for (unsigned int i = 0; i < m_clients.size(); i++)
	{
		if (m_clients[i].name.find(client_name) != std::string::npos)
		{
			// Free the ports
			liberatePorts(m_clients[i].publishing_port, m_clients[i].subscribing_port);

			// Found client object
			m_clients.erase(m_clients.begin() + i);
		}
	}

	return true;
}


std::pair<int, int> PortManager::storeClient(std::string client_name)
{
	// Store values into vector of client objects
	ClientObj new_client;

	int subscriber_port = returnAvailablePort();
	int publisher_port = subscriber_port + 1;

	new_client.name = client_name;
	new_client.publishing_port = subscriber_port;
	new_client.subscribing_port = publisher_port;

	m_clients.push_back(new_client);
	std::pair<int, int> ports;
	ports.first = subscriber_port;
	ports.second = publisher_port;

	return ports;

}


ClientObj PortManager::getClient(std::string client_name)
{
	// Store values into vector of client objects
	ClientObj empty_client;

	for (unsigned int i = 0; i < m_clients.size(); i++)
	{
		if (m_clients[i].name.find(client_name) != std::string::npos) 
		{
			// Found client object
			return m_clients[i];
		}
	}

	return empty_client;
}

bool PortManager::isConnected(std::string client_name)
{

	for (unsigned int i = 0; i < m_clients.size(); i++)
	{
		if (m_clients[i].name.find(client_name) != std::string::npos)
		{
			// Found client object
			return true;
		}
	}

	return false;
}


PortManager::PortManager(int starting_port, int max_number_clients)
{
	m_starting_port = starting_port;
	m_max_clients = max_number_clients;


	for (unsigned int port_number = starting_port; port_number < (starting_port + (max_number_clients * 2)); port_number++)
	{
		m_map_taken_ports[port_number] = 0;
	}
	
}

PortManager::~PortManager()
{

}