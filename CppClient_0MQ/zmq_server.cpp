#include <future>
#include <iostream>
#include <string>

#include "zmq.hpp"
#include "zmq_addon.hpp"

// USeful links:
// https://stackoverflow.com/questions/10901240/zeromq-how-to-access-tcp-message-in-c


void PublisherThread(zmq::context_t* ctx) {
    //  Prepare publisher
    zmq::socket_t publisher(*ctx, zmq::socket_type::pub);
    
    publisher.bind("tcp://127.0.0.1:5555");

    // Give the subscribers a chance to connect, so they don't lose any messages
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    while (true) 
    {
        std::cout << "Sending messages.." << std::endl;
        //  Write three messages, each with an envelope and content
        publisher.send(zmq::str_buffer("A"), zmq::send_flags::sndmore);
        publisher.send(zmq::str_buffer("Message in A envelope"));
        publisher.send(zmq::str_buffer("B"), zmq::send_flags::sndmore);
        publisher.send(zmq::str_buffer("Message in B envelope"));
        publisher.send(zmq::str_buffer("C"), zmq::send_flags::sndmore);
        publisher.send(zmq::str_buffer("Message in C envelope"));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void SubscriberThread1(zmq::context_t* ctx) {
    //  Prepare subscriber
    zmq::socket_t subscriber(*ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://127.0.0.1:5555");

    //  Thread2 opens "A" and "B" envelopes
    subscriber.set(zmq::sockopt::subscribe, "A");
    subscriber.set(zmq::sockopt::subscribe, "B");

    while (1) {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result =
            zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
        assert(result && "recv failed");
        assert(*result == 2);

        std::cout << "Thread2: [" << recv_msgs[0].to_string() << "] "
            << recv_msgs[1].to_string() << std::endl;
    }
}

void SubscriberThread2(zmq::context_t* ctx) {
    //  Prepare our context and subscriber
    zmq::socket_t subscriber(*ctx, zmq::socket_type::sub);
    subscriber.connect("tcp://127.0.0.1:5555");

    //  Thread3 opens ALL envelopes
    subscriber.set(zmq::sockopt::subscribe, "");

    while (1) {
        // Receive all parts of the message
        std::vector<zmq::message_t> recv_msgs;
        zmq::recv_result_t result =
            zmq::recv_multipart(subscriber, std::back_inserter(recv_msgs));
        assert(result && "recv failed");
        assert(*result == 2);

        std::cout << "Thread3: [" << recv_msgs[0].to_string() << "] "
            << recv_msgs[1].to_string() << std::endl;
    }
}

int main() {
    /*
     * No I/O threads are involved in passing messages using the inproc transport.
     * Therefore, if you are using a ØMQ context for in-process messaging only you
     * can initialise the context with zero I/O threads.
     *
     * Source: http://api.zeromq.org/4-3:zmq-inproc
     */
    zmq::context_t ctx(1);
    auto thread1 = std::async(std::launch::async, PublisherThread, &ctx);

    // Give the publisher a chance to bind, since inproc requires it
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    //auto thread2 = std::async(std::launch::async, SubscriberThread1, &ctx);
    //auto thread3 = std::async(std::launch::async, SubscriberThread2, &ctx);
    thread1.wait();
    //thread2.wait();
    //thread3.wait();



    int error_code = zmq_errno();
    std::cout << "Error code: " << error_code << std::endl;
    /*
     * Output:
     *   An infinite loop of a mix of:
     *     Thread2: [A] Message in A envelope
     *     Thread2: [B] Message in B envelope
     *     Thread3: [A] Message in A envelope
     *     Thread3: [B] Message in B envelope
     *     Thread3: [C] Message in C envelope
     */
}
