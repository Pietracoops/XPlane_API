#pragma once


#ifndef CLIENTMANAGER_SRC_CLIENTMANAGER_H_
#define CLIENTMANAGER_SRC_CLIENTMANAGER_H_

#define NOMINMAX

#include <future>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <sstream>
#include <functional>
#include <mutex>
#include <shared_mutex>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "PortManager.h"

#include "XPlaneBeaconListener.h"
#include "XPlaneUDPClient.h"
#include "XPUtils.h"

// globals



void listenForClients(XPlaneUDPClient& xp);
void attachToClient(zmq::context_t* ctx, std::string topic, int thread_number, int port_number, XPlaneUDPClient* xp);

class ClientManager
{
private:

	bool m_client_terminated;
	

public:
	ClientManager();
	~ClientManager();
	bool terminate();
	void run();

};
#endif /* CLIENTMANAGER_SRC_CLIENTMANAGER_H_ */