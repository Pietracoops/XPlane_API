#include "ClientManager.h"

#include <atomic>
#include <functional>
#include <future>
#include <iostream>

void ClientManager::receiverCallbackFloat(std::string dataref, float value)
{
    std::cout << "receiverCallbackFloat got [" << dataref << "] and [" << value << "]" << std::endl;
    m_DataRefValuesMap[dataref] = std::to_string(value);
}

void ClientManager::receiverCallbackString(std::string dataref, std::string value)
{
    std::cout << "receiverCallbackString got [" << dataref << "] and [" << value << "]" << std::endl;
    m_DataRefValuesMap[dataref] = value;
}

void ClientManager::receiverBeaconCallback(XPlaneBeaconListener::XPlaneServer server, bool exists)
{
    std::cout << "receiverBeaconCallback got [" << server.toString() << " is " << (exists ? "alive" : "dead") << "]" << std::endl;
    m_Host = server.host;
    m_Port = server.receivePort;
    m_Found = true;
}

std::string& ClientManager::getDataRefFromMap(std::string& dref)
{
    return m_DataRefValuesMap[dref];
}

void ClientManager::attachToClient(const std::string& topic, size_t publisher_index, size_t subscriber_index)
{
    /*
    // Pass socket by address to each thread
    std::string subscriber_connection_string = "tcp://127.0.0.1:" + std::to_string(port_number);
    std::string publisher_connection_string = "tcp://127.0.0.1:" + std::to_string(port_number + 1);
    
    // Prepare our context and subscriber
    zmq::socket_t subscriber(m_Context, zmq::socket_type::sub);
    subscriber.connect(subscriber_connection_string);
    
    // Prepare our publisher
    zmq::socket_t publisher(m_Context, zmq::socket_type::pub);
    publisher.bind(publisher_connection_string);

    // Opens ALL envelopes
    subscriber.set(zmq::sockopt::subscribe, topic);
    */
    
    m_Subscribers[subscriber_index].set(zmq::sockopt::subscribe, topic);

    while (m_Running)
    {
        //std::cout << "STARTING UP" << std::endl;
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t  result = zmq::recv_multipart(m_Subscribers[subscriber_index], std::back_inserter(recv_msgs), zmq::recv_flags::dontwait);
        if (!result) continue;
        //assert(result && "recv failed");
        //assert(*result == 2);

        if (recv_msgs.size() != 4)
        {
            // Respond with malformed message and continue
            std::cout << "====================Too small==========================" << recv_msgs.size() << std::endl;
            //for (int i = 0; i < recv_msgs.size(); i++)
            //{
            //    std::cout << recv_msgs[i].to_string() << std::endl;
            //}
            //std::cout << "====================Too small==========================" << recv_msgs.size() << std::endl;
            continue;
        }

        std::string topic_found = recv_msgs[0].to_string();
        std::string command = recv_msgs[1].to_string();
        std::string dref = recv_msgs[2].to_string();
        std::string value = recv_msgs[3].to_string();

        if (command == "read")
        {
            
            std::string response = getDataRefFromMap(dref);
            std::cout << "READ COMMAND FOUND - " << topic << ": [" << recv_msgs[0].to_string() << "] " << recv_msgs[1].to_string() << " VALUE: " << response << std::endl;

            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer(response));

        }

        if (command == "set")
        {

            float r = std::stof(value);
            m_XPlaneClient->setDataRef(dref, r);

            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer("Received"));

        }

        if (command == "command")
        {
            m_XPlaneClient->sendCommand(dref);

            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer("Received"));

        }

        if (command == "Disconnection")
        {
            m_Publishers[publisher_index].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[publisher_index].send(zmq::buffer("Received"));
            break;
        }

    
        std::cout << "Thread topic - " << topic << ": [" << recv_msgs[0].to_string() << "] " << recv_msgs[1].to_string() << std::endl;
    
    
    }

    std::unique_lock lock(m_Mutex);
    m_ClientsToRemove.push(Client(topic, publisher_index, subscriber_index));
    std::cout << "Terminating connection for topic: " << topic << std::endl;
}

void ClientManager::listenForClients()
{
    // Main thread to listen (subscriber) and send m_Port to client (publisher)
    std::cout << std::endl;
    std::cout << "Running Listener" << std::endl;
    // Vector to hold all threads for each new client
    std::vector<std::future<void>> threads;

    auto [portPublisher, pport] = bind();
    auto [subscriberOfClients, sport] = connect();
    
    // Opens ALL envelopes
    // Reader
    m_Subscribers[subscriberOfClients].set(zmq::sockopt::subscribe, "");

    while (m_Running)
    {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result = zmq::recv_multipart(m_Subscribers[subscriberOfClients], std::back_inserter(recv_msgs), zmq::recv_flags::dontwait);
        if (!result) continue;

        std::string topic = recv_msgs[0].to_string();
    
        if (std::find(m_ClientTopics.begin(), m_ClientTopics.end(), topic) == m_ClientTopics.end())
        {
            /*
            auto [new_subscribing_port, new_publishing_port] = m_PortManager.storeClient(topic);

            std::cout << "New Client Connected : " << topic << " - on m_Port " << new_subscribing_port << " and " << new_publishing_port << std::endl;

            // Send back the new m_Port they should connect to for their personal connection
            publisher.send(zmq::buffer(topic), zmq::send_flags::sndmore);
            publisher.send(zmq::buffer(std::to_string(new_subscribing_port)));
            */

            m_ClientTopics.emplace_back(topic);

            auto [newPublisher, newPublisherPort] = bind();
            auto [newSubscriber, newSubscriberPort] = connect();

            std::cout << "New client connected: " << topic << " - on port " << newPublisherPort << " and " << newSubscriberPort << std::endl;

            m_Publishers[portPublisher].send(zmq::buffer(topic), zmq::send_flags::sndmore);
            m_Publishers[portPublisher].send(zmq::buffer(std::to_string(newSubscriberPort)));
            
            threads.push_back(std::async(std::launch::async, &ClientManager::attachToClient, this, std::ref(topic), newPublisher, newSubscriber)); // push it back into the thread vector
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
    }
    
    std::cout << "Terminating Listener" << std::endl;
}

// Prepare our context and the ClientManager
ClientManager::ClientManager() : m_ClientTerminated(false), m_PortManager(s_StaringPort, 20), m_Context(1)
{
    std::cout << "Initializing Client Manager" << std::endl;
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

    //std::cout << "Press Control-C to abort." << std::endl;
    // Search for XPlane
    while (!m_Found) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000ms
        std::cout << "Looking for XPlane..." << std::endl;

    }
    std::cout << "Found server " << m_Host << ":" << m_Port << std::endl;


    // Definitions
    std::string dataRefsFileName = "Subscriptions.txt";
    std::unordered_map<std::string, int> dataRefsMap;

    // Init the Xplane UDP Client
    std::cout << "Initializing XPlane UDP Client" << std::endl;
    m_XPlaneClient = new XPlaneUDPClient(m_Host, m_Port, 
        std::bind(&ClientManager::receiverCallbackFloat, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&ClientManager::receiverCallbackString, this, std::placeholders::_1, std::placeholders::_2)
    );

    m_XPlaneClient->setDebug(0);


    // Read in the dataref Values
    int result = readDataRefsFromFile(dataRefsFileName, dataRefsMap);
    if (result != 0) std::cout << "Subscriptions.txt missing or unable to open." << std::endl;


    // Create subscriptions
    for (auto const& [key, val] : dataRefsMap)
    {
        std::cout << "Creating subscription for " << key << " with min frequency of " << val << std::endl;
        m_XPlaneClient->subscribeDataRef(key, val);
    }

    //m_XPlaneClient->subscribeDataRef("sim/aircraft/view/acf_descrip[0][40]", 1);
    //m_XPlaneClient->subscribeDataRef("sim/cockpit2/engine/actuators/throttle_ratio[0]", 10);
    


    //m_XPlaneClient->sendCommand("sim/flight_controls/flaps_down");
    //m_XPlaneClient->sendCommand("sim/flight_controls/flaps_down");
    //
    //float r = 0;
    //float i = 0.01f;
    //
    //
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 20));
    //
    //while (1) {
    //    std::this_thread::sleep_for(std::chrono::milliseconds(100 * 1));
    //
    //    m_XPlaneClient->setDataRef("sim/multiplayer/controls/engine_throttle_request[0]", r);
    //    r += i;
    //
    //    if (r > 1) {
    //        i = -0.01f;
    //    }
    //    else if (r < 0) {
    //        i = 0.01f;
    //    }
    //
    //}

    listenForClients();
    m_ClientTerminated = true;
}

bool ClientManager::terminate()
{
    for (size_t i = 0; i < m_Publishers.size(); ++i) unbind(i);
    for (size_t i = 0; i < m_Subscribers.size(); ++i) disconnect(i);

    m_Running = false;
    return m_ClientTerminated;
}

size_t ClientManager::storeInDeque(zmq::socket_type socket_type, std::vector<size_t>& free, std::deque<zmq::socket_t>& deque, std::deque<std::string>& s_deque)
{
    if (!free.empty())
    {
        size_t freeSlot = free.back();
        deque[freeSlot] = zmq::socket_t(m_Context, socket_type);
        s_deque[freeSlot] = "dummy";
        free.pop_back();
        return freeSlot;
    }
    s_deque.emplace_back("dummy");
    deque.emplace_back(m_Context, socket_type);
    return deque.size() - 1;
}

std::pair<size_t, unsigned int> ClientManager::bind()
{
    size_t publisher = storeInDeque(zmq::socket_type::pub, m_UnusedPublishers, m_Publishers, m_PublishersAddress);

    int port = 0;
    int returnValue = -1;
    std::string fullAddress = "dummy";
    while (returnValue != 0)
    {
        port = m_PortManager.returnAvailablePort();
        fullAddress = m_Address + std::to_string(port);
        returnValue = m_Publishers[publisher].bind(fullAddress);
        m_PortManager.occupyPort(port);
        std::cout << "Binding to port: " << port << std::endl;
    }
    m_PublishersAddress[publisher] = fullAddress;

    return std::make_pair(publisher, port);
}

void ClientManager::unbind(size_t publisher_index)
{
    m_Publishers[publisher_index].unbind(m_PublishersAddress[publisher_index]);
    m_UnusedPublishers.emplace_back(publisher_index);
}

std::pair<size_t, unsigned int> ClientManager::connect()
{
    size_t subscriber = storeInDeque(zmq::socket_type::sub, m_UnusedSubscribers, m_Subscribers, m_SubscribersAddress);

    int port = 0;
    int returnValue = -1;
    std::string fullAddress = "dummy";
    while (returnValue != 0)
    {
        port = m_PortManager.returnAvailablePort();
        fullAddress = m_Address + std::to_string(port);
        returnValue = m_Subscribers[subscriber].connect(m_Address + std::to_string(port));
        m_PortManager.occupyPort(port);
        std::cout << "Connecting to port: " << port << std::endl;
    }
    m_SubscribersAddress[subscriber] = fullAddress;

    return std::make_pair(subscriber, port);
}

void ClientManager::disconnect(size_t subscriber_index)
{
    m_Publishers[subscriber_index].disconnect(m_PublishersAddress[subscriber_index]);
    m_UnusedPublishers.emplace_back(subscriber_index);
}