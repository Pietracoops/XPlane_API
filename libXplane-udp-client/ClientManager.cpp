#include "ClientManager.h"

bool found = false;
std::string host;
uint16_t port;
std::atomic<bool> running = false;
std::shared_mutex mutex_;


std::map<std::string, std::string> dataRefValuesMap;

void receiverCallbackFloat(std::string dataref, float value)
{
    std::cout << "receiverCallbackFloat got [" << dataref << "] and [" << value << "]" << std::endl;
    std::unique_lock lock(mutex_);
    dataRefValuesMap[dataref] = std::to_string(value);
}

void receiverCallbackString(std::string dataref, std::string value)
{
    std::cout << "receiverCallbackString got [" << dataref << "] and [" << value << "]" << std::endl;
    std::unique_lock lock(mutex_);
    dataRefValuesMap[dataref] = value;
}

// our callback for changed values.

void receiverBeaconCallback(XPlaneBeaconListener::XPlaneServer server, bool exists) 
{
    std::cout << "receiverBeaconCallback got [" << server.toString() << " is " << (exists ? "alive" : "dead") << "]" << std::endl;
    host = server.host;
    port = server.receivePort;
    found = true;
}


void attachToClient(zmq::context_t* ctx, std::string topic, int thread_number)
{

    // Vector to hold all threads for each new client
    std::vector<std::future<void>> threads;
    
    //  Prepare our context and subscriber
    zmq::socket_t subscriber(*ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://127.0.0.1:5555");
    
    // Prepare our publisher
    zmq::socket_t publisher(*ctx, zmq::socket_type::pub);
    publisher.bind("tcp://127.0.0.1:5556");

    //  Opens ALL envelopes
    subscriber.set(zmq::sockopt::subscribe, topic);
    

    while (running)
    {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t  result = zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs), zmq::recv_flags::dontwait);
        if (!result)
        {
            continue;
        }
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
            
            std::shared_lock lock(mutex_);
            std::string response = dataRefValuesMap[dref];
            std::cout << "READ COMMAND FOUND - " << topic << ": [" << recv_msgs[0].to_string() << "] " << recv_msgs[1].to_string() << " VALUE: " << response << std::endl;
            std::shared_lock unlock(mutex_);

            publisher.send(zmq::buffer(topic), zmq::send_flags::sndmore);
            publisher.send(zmq::buffer(response));

        }
    
        std::cout << "Thread topic - " << topic << ": [" << recv_msgs[0].to_string() << "] " << recv_msgs[1].to_string() << std::endl;
    
    
    }

    std::cout << "Terminating connection for topic: " << topic << std::endl;
}

void listenForClients(XPlaneUDPClient& xp)
{


    std::cout << std::endl;
    std::cout << "Running Listener" << std::endl;
    // Vector to hold all threads for each new client
    std::vector<std::future<void>> threads;
    std::map<std::string, int> topics;
    int thread_number = 0;
    
    zmq::context_t ctx(1);
    
    //  Prepare our context and subscriber
    zmq::socket_t subscriber(ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://127.0.0.1:5555");
    
    //  Opens ALL envelopes
    subscriber.set(zmq::sockopt::subscribe, "");
    

    while (running)
    {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result = zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs), zmq::recv_flags::dontwait);
        if (!result)
        {
            continue;
        }
        //assert(result && "recv failed");
        //assert(*result == 2);
        std::string topic = recv_msgs[0].to_string();
    
        if (topics[topic] == 0)
        {
            std::cout << "New Client Connected : " << topic << std::endl;
            topics[topic] = 1; // Store it in the map
            threads.push_back(std::async(std::launch::async, attachToClient, &ctx, topic, thread_number)); // push it back into the thread vector
            thread_number++;
        }
    
    }
    
    std::cout << "Terminating Listener" << std::endl;
}


ClientManager::ClientManager()
{
    
    std::cout << "Initializing Client Manager" << std::endl;
    m_client_terminated = false;

}

ClientManager::~ClientManager()
{
    terminate();
}

void ClientManager::run()
{
    running = true;

    XPlaneBeaconListener::getInstance()->registerNotificationCallback(std::bind(receiverBeaconCallback, std::placeholders::_1, std::placeholders::_2));
    XPlaneBeaconListener::getInstance()->setDebug(0);

    std::cout << "Press Control-C to abort." << std::endl;
    // Search for XPlane
    while (!found) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000ms
        std::cout << "Looking for XPlane..." << std::endl;

    }
    std::cout << "Found server " << host << ":" << port << std::endl;


    // Definitions
    std::string dataRefsFileName = "Subscriptions.txt";
    std::map<std::string, int> dataRefsMap;

    // Init the Xplane UDP Client
    std::cout << "Initializing XPlane UDP Client" << std::endl;
    XPlaneUDPClient xp(host, port, std::bind(receiverCallbackFloat, std::placeholders::_1, std::placeholders::_2),
        std::bind(receiverCallbackString, std::placeholders::_1, std::placeholders::_2));

    xp.setDebug(0);


    // Read in the dataref Values
    int result = readDataRefsFromFile(dataRefsFileName, dataRefsMap);
    if (result != 0)
    {
        std::cout << "Subscriptions.txt missing or unable to open." << std::endl;
    }


    // Create subscriptions
    for (auto const& [key, val] : dataRefsMap)
    {
        std::cout << "Creating subscription for " << key << " with min frequency of " << val << std::endl;
        xp.subscribeDataRef(key, val);
    }

    //xp.subscribeDataRef("sim/aircraft/view/acf_descrip[0][40]", 1);
    //xp.subscribeDataRef("sim/cockpit2/engine/actuators/throttle_ratio[0]", 10);
    


    //xp.sendCommand("sim/flight_controls/flaps_down");
    //xp.sendCommand("sim/flight_controls/flaps_down");
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
    //    xp.setDataRef("sim/multiplayer/controls/engine_throttle_request[0]", r);
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

    listenForClients(xp);
    m_client_terminated = true;
}

bool ClientManager::terminate()
{
    running = false;
    return m_client_terminated;
}