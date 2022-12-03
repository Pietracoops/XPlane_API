#include "ClientManager.h"

#include <functional>
#include <future>
#include <iostream>

#include <yaml-cpp/yaml.h>

#include "Log.h"

void ClientManager::receiverCallbackFloat(std::string dataref, float value)
{
    LOG_INFO("receiverCallbackFloat got [{0}] and [{1}]", dataref, value);
    m_DataRefs[dataref] = DataRef(std::to_string(value), std::chrono::steady_clock::now());
}

void ClientManager::receiverCallbackString(std::string dataref, std::string value)
{
    LOG_INFO("receiverCallbackFloat got [{0}] and [{1}]", dataref, value);
    m_DataRefs[dataref] = DataRef(value, std::chrono::steady_clock::now());
}

void ClientManager::receiverBeaconCallback(XPlaneBeaconListener::XPlaneServer server, bool exists)
{
    LOG_INFO("receiverBeaconCallback got [{0} is {1}]", server.toString(), exists ? "alive" : "dead");
    m_Host = server.host;
    m_Port = server.receivePort;
    m_Found = true;
}

void ClientManager::setDataRef(const std::string& dataref, const std::string& value, std::string& response)
{
    float v = std::stof(value);
    m_XPlaneClient->setDataRef(dataref, v);
}

void ClientManager::terminateWriter(const std::string& topic)
{
    if (m_Writer.Topic == topic)
    {
        m_Writer.IsCockpitFree = true;
        m_Writer.Topic.clear();
    }
}

void ClientManager::attachToClient(std::string topic, size_t publisher_index, size_t subscriber_index)
{    
    m_Subscribers[subscriber_index].set(zmq::sockopt::subscribe, topic);

    while (m_Running)
    {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t  result = zmq::recv_multipart(m_Subscribers[subscriber_index], std::back_inserter(recv_msgs), zmq::recv_flags::dontwait);
        if (!result) continue;

        if (recv_msgs.size() != 4)
        {
            // Respond with malformed message and continue
            LOG_WARN("Message: [{0}] is too small", recv_msgs.size());
            continue;
        }

        std::string topic_found = recv_msgs[0].to_string();
        std::string command = recv_msgs[1].to_string();
        std::string label = recv_msgs[2].to_string();
        std::string value = recv_msgs[3].to_string();

        if (command == "read")
        {
            std::string response;
            std::chrono::nanoseconds timeElaspedNanoSeconds;
            if (m_LabelsToTag.find(label) != m_LabelsToTag.end())
            {
                response = getDataRef(label).Value;
                timeElaspedNanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - getDataRef(label).LastUpdateTime);
            }
            else response = "Error: Server is currently not subscribed to " + label;

            LOG_INFO("READ COMMAND FOUND - {0}: [{1}] {2} VALUE: {3}", topic, recv_msgs[0].to_string(), recv_msgs[1].to_string(), response);

            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer(response), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer( std::to_string(timeElaspedNanoSeconds.count() / 1e9 ) ));
        }

        if (command == "set")
        {
            std::string response = "Received";
            if (m_LabelsToTag.find(label) != m_LabelsToTag.end())
            {
            
                if (std::regex_search(label, m_IpclLabels)) m_DataRefs[m_LabelsToTag[label]] = DataRef(value, std::chrono::steady_clock::now());
  
                else
                {
                    if (m_Writer.IsCockpitFree)
                    {
                        m_Writer.IsCockpitFree = false;
                        m_Writer.Topic = topic;
                        setDataRef(m_LabelsToTag[label], value, response);
                    }
                    else if (m_Writer.Topic == topic) setDataRef(m_LabelsToTag[label], value, response);
                    else response = "Error: Cockpit is already in use! Please wait for termination of the current writer";
                }

            }
            else response = "Error: Server is currently not subscribed to " + label;

            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer(response));
        }

        if (command == "terminate")
        {
            terminateWriter(topic);


            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer("Received"));
        }

        if (command == "command")
        {
            if (!std::regex_search(label, m_IpclLabels)) m_XPlaneClient->sendCommand(m_LabelsToTag[label]);

            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer("Received"));
        }

        if (command == "Disconnection")
        {
            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer("Received"));
            break;
        }

        LOG_INFO("Thread topic - {0}: [{1}] {2}", topic, recv_msgs[0].to_string(), recv_msgs[1].to_string());
    }

    terminateWriter(topic);

    std::unique_lock lock(m_Mutex);
    m_ClientsToRemove.push(Client(topic, publisher_index, subscriber_index));
    LOG_INFO("Terminating connection for topic: {0}", topic);
}

void ClientManager::listenForClients()
{
    // Main thread to listen (subscriber) and send m_Port to client (publisher)
    LOG_INFO("Running Listener");
    // Vector to hold all threads for each new client
    std::vector<std::future<void>> threads;

    auto [portPublisher, pport] = bind();
    auto [subscriberOfClients, sport] = connect();
    
    // Opens ALL envelopes
    // Reader
    m_Subscribers[subscriberOfClients].set(zmq::sockopt::subscribe, "");

    LOG_INFO("Connect to server by assigning client's publisher port to: {0}", sport);
    LOG_INFO("Connect to server by assigning client's subscriber port to: {0}", pport);

    for (auto const& [label, tag] : m_LabelsToTag) m_DataRefLogger << label << ", ";
    m_DataRefLogger << std::endl;

    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
    while (m_Running)
    {
        // Log Data
        std::chrono::seconds secondsElasped = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - time);
        if (secondsElasped.count() > (long long)m_LoggingFrequency)
        {
            time = std::chrono::steady_clock::now();
            for (auto const& [label, tag] : m_LabelsToTag) m_DataRefLogger << getDataRef(label).Value << ", ";
            m_DataRefLogger << std::endl;
        }

        // Remove topic
        // Unbind and diconnect sockets used to communicate to disconnected clients
        std::unique_lock lock(m_Mutex);
        while (!m_ClientsToRemove.empty())
        {
            for (size_t i = 0; i < m_ClientTopics.size(); ++i)
            {
                if (m_ClientTopics[i] == m_ClientsToRemove.top().Topic)
                {
                    m_ClientTopics.erase(m_ClientTopics.begin() + i);
                    break;
                }
            }
            unbind(m_ClientsToRemove.top().Publisher);
            disconnect(m_ClientsToRemove.top().Subscriber);

            m_ClientsToRemove.pop();
        }

        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result = zmq::recv_multipart(m_Subscribers[subscriberOfClients], std::back_inserter(recv_msgs), zmq::recv_flags::dontwait);
        if (!result) continue;

        std::string topic = recv_msgs[0].to_string();

        LOG_INFO("Received topic: {0} with command: {1}", topic, recv_msgs[1].to_string());
    
        if (std::find(m_ClientTopics.begin(), m_ClientTopics.end(), topic) == m_ClientTopics.end())
        {
            m_ClientTopics.emplace_back(topic);

            auto [newPublisher, newPublisherPort] = bind();
            auto [newSubscriber, newSubscriberPort] = connect();

            LOG_INFO("New client connected: {0} - on publishing port {1} and subscription port {2}", topic, newPublisherPort, newSubscriberPort);

            m_Publishers[portPublisher].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[portPublisher].send(zmq::buffer(std::to_string(newSubscriberPort)), zmq::send_flags::sndmore);
            m_Publishers[portPublisher].send(zmq::buffer(std::to_string(newPublisherPort)));
            
            threads.push_back(std::async(std::launch::async, &ClientManager::attachToClient, this, m_ClientTopics.back(), newPublisher, newSubscriber)); // push it back into the thread vector
        }
    }
    
    LOG_INFO("Terminating Listener");
}

// Prepare our context and the ClientManager
ClientManager::ClientManager() : m_ClientTerminated(false), m_PortManager(s_StaringPort, 20), m_Context(1), m_DataRefLogger("data.log")
{
    LOG_INFO("Initializing Client Manager");
}

ClientManager::~ClientManager()
{
    terminate();
}

void ClientManager::run()
{
    m_Running = true;

    XPlaneBeaconListener::getInstance()->registerNotificationCallback(std::bind(&ClientManager::receiverBeaconCallback, this, std::placeholders::_1, std::placeholders::_2));
    XPlaneBeaconListener::getInstance()->setDebug(0);

    // Search for XPlane
    while (!m_Found) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000ms
        LOG_INFO("Looking for XPlane...");

    }
    LOG_INFO("Found server {0}:{1}", m_Host, m_Port);

    // Definitions
    std::string dataRefsFileName = "Subscriptions.yaml";
    std::unordered_map<std::string, int> dataRefsMap;

    // Init the Xplane UDP Client
    LOG_INFO("Initializing XPlane UDP Client");
    m_XPlaneClient = new XPlaneUDPClient(m_Host, m_Port, 
        std::bind(&ClientManager::receiverCallbackFloat, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&ClientManager::receiverCallbackString, this, std::placeholders::_1, std::placeholders::_2)
    );

    m_XPlaneClient->setDebug(0);

    // Read in the dataref Values
    int result = readDataRefsFromFile(dataRefsFileName, dataRefsMap);
    if (result != 0) LOG_ERROR("Subscriptions.txt missing or unable to open file");
    m_IpclLabels.assign("ipcl/");

    // Create subscriptions
    for (auto const& [key, val] : dataRefsMap)
    {
        LOG_INFO("Creating subscription for {0} with min frequency of {1}", key, val);
        if (!std::regex_search(key, m_IpclLabels))
        {
            m_XPlaneClient->subscribeDataRef(key, val);
        }
        
    }

    listenForClients();
    m_ClientTerminated = true;
}

bool ClientManager::terminate()
{
    m_DataRefLogger.close();
    for (size_t i = 0; i < m_Publishers.size(); ++i) unbind(i);
    for (size_t i = 0; i < m_Subscribers.size(); ++i) disconnect(i);

    m_Running = false;
    return m_ClientTerminated;
}

size_t ClientManager::storeInDeque(zmq::socket_type socket_type, std::vector<size_t>& free, 
    std::deque<zmq::socket_t>& deque, 
    std::deque<std::string>& s_deque,
    std::deque<unsigned int>& p_deque)
{
    if (!free.empty())
    {
        size_t freeSlot = free.back();
        deque[freeSlot] = zmq::socket_t(m_Context, socket_type);
        s_deque[freeSlot] = "dummy";
        p_deque[freeSlot] = 0;
        free.pop_back();
        return freeSlot;
    }
    s_deque.emplace_back("dummy");
    p_deque.emplace_back(0);
    deque.emplace_back(m_Context, socket_type);
    return deque.size() - 1;
}

std::pair<size_t, unsigned int> ClientManager::bind()
{
    size_t publisher = storeInDeque(zmq::socket_type::pub, m_UnusedPublishers, m_Publishers, m_PublishersAddress, m_PublishersPorts);

    int port = 0;
    int returnValue = -1;
    std::string fullAddress = "dummy";
    while (returnValue != 0)
    {
        port = m_PortManager.returnAvailablePort();
        fullAddress = m_Address + std::to_string(port);
        returnValue = m_Publishers[publisher].bind(fullAddress);
        m_PortManager.occupyPort(port);
        LOG_INFO("New publisher bound to port: {0}", port);
    }
    m_PublishersAddress[publisher] = fullAddress;
    m_PublishersPorts[publisher] = port;

    return std::make_pair(publisher, port);
}

void ClientManager::unbind(size_t publisher_index)
{
    m_Publishers[publisher_index].unbind(m_PublishersAddress[publisher_index]);
    m_PortManager.liberatePort(m_PublishersPorts[publisher_index]);
    m_UnusedPublishers.emplace_back(publisher_index);
}

std::pair<size_t, unsigned int> ClientManager::connect()
{
    size_t subscriber = storeInDeque(zmq::socket_type::sub, m_UnusedSubscribers, m_Subscribers, m_SubscribersAddress, m_SubscriberPorts);

    int port = 0;
    int returnValue = -1;
    std::string fullAddress = "dummy";
    while (returnValue != 0)
    {
        port = m_PortManager.returnAvailablePort();
        fullAddress = m_Address + std::to_string(port);
        returnValue = m_Subscribers[subscriber].connect(m_Address + std::to_string(port));
        m_PortManager.occupyPort(port);
        LOG_INFO("New subscriber connected to port: {0}", port);
    }
    m_SubscribersAddress[subscriber] = fullAddress;
    m_SubscriberPorts[subscriber] = port;

    return std::make_pair(subscriber, port);
}

void ClientManager::disconnect(size_t subscriber_index)
{
    m_Subscribers[subscriber_index].disconnect(m_SubscribersAddress[subscriber_index]);
    m_PortManager.liberatePort(m_SubscriberPorts[subscriber_index]);
    m_UnusedSubscribers.emplace_back(subscriber_index);
}

int ClientManager::readDataRefsFromFile(const std::string& fileName, std::unordered_map<std::string, int>& map)
{
    YAML::Node subscriptions = YAML::LoadFile(fileName);
    for (YAML::const_iterator it = subscriptions.begin(); it != subscriptions.end(); ++it)
    {
        std::string label = it->first.as<std::string>();
        std::string tag = it->second["tag"].as<std::string>();
        int frequency = it->second["frequency"].as<int>();
        LOG_INFO("Label: {0}, Tag: {1}, Frequency: {2}", label, tag, frequency);

        m_LabelsToTag[label] = tag;

        map[tag] = frequency;
    }

    return 0;
}
