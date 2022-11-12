#pragma once


#ifndef CLIENTMANAGER_SRC_CLIENTMANAGER_H_
#define CLIENTMANAGER_SRC_CLIENTMANAGER_H_

#define NOMINMAX

#include <chrono>
#include <deque>
#include <shared_mutex>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>

#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "PortManager.h"

#include "XPlaneBeaconListener.h"
#include "XPlaneUDPClient.h"
#include "XPUtils.h"

class ClientManager
{
private:
	bool m_ClientTerminated;
	PortManager m_PortManager;
	zmq::context_t m_Context;

	bool m_Found = false;
	std::string m_Host;
	uint16_t m_Port = 0;
	std::atomic<bool> m_Running = false;

	struct DataRef
	{
		std::string Value;
		std::chrono::steady_clock::time_point LastUpdateTime;

		DataRef() = default;
		DataRef(const std::string& value, const std::chrono::steady_clock::time_point& lastUpdateTime) : Value(value), LastUpdateTime(lastUpdateTime) { }
	};

	std::unordered_map<std::string, DataRef> m_DataRefs;

	XPlaneUDPClient* m_XPlaneClient = nullptr;

	std::vector<size_t> m_UnusedPublishers;
	std::deque<zmq::socket_t> m_Publishers;
	std::deque<std::string> m_PublishersAddress;
	std::deque<unsigned int> m_PublishersPorts;

	std::vector<size_t> m_UnusedSubscribers;
	std::deque<zmq::socket_t> m_Subscribers;
	std::deque<std::string> m_SubscribersAddress;
	std::deque<unsigned int> m_SubscriberPorts;

	std::vector<std::string> m_ClientTopics;
	std::shared_mutex m_Mutex;

	struct Writer
	{
		bool IsCockpitFree = true;

		std::string Topic;
	};

	Writer m_Writer;

	const std::string m_Address = "tcp://127.0.0.1:";

	std::regex m_ipcl_labels;

	static constexpr unsigned int s_StaringPort = 5555;

	struct Client
	{
		std::string Topic;
		size_t Publisher;
		size_t Subscriber;

		Client(const std::string& t, size_t p, size_t s) : Topic(t), Publisher(p), Subscriber(s) { }
	};

	std::stack<Client> m_ClientsToRemove;

public:
	ClientManager();
	~ClientManager();

	void run();

	bool terminate();

private:
	void listenForClients();
	void attachToClient(std::string topic, size_t publisher_index, size_t subscriber_index);

	void receiverCallbackFloat(std::string dataref, float value);
	void receiverCallbackString(std::string dataref, std::string value);
	void receiverBeaconCallback(XPlaneBeaconListener::XPlaneServer server, bool exists);

	void setDataRef(const std::string& dataref, const std::string& value, std::string& response);
	void terminateWriter(const std::string& topic);

	size_t storeInDeque(zmq::socket_type socket_type, std::vector<size_t>& free, 
		std::deque<zmq::socket_t>& deque, 
		std::deque<std::string>& s_deque,
		std::deque<unsigned int>& p_deque);

	// Bind to a socket of the server
	// Returns a index to a publisher and the port associated to it
	std::pair<size_t, unsigned int> bind();

	void unbind(size_t publisher_index);

	// Connect to a socket of the server
	// Returns a index to a subscriber and the port associated to it
	std::pair<size_t, unsigned int> connect();

	void disconnect(size_t subscriber_index);

};
#endif /* CLIENTMANAGER_SRC_CLIENTMANAGER_H_ */