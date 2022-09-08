import zmq
import time

# Initialize a zeromq context
context = zmq.Context()

# # Set up a channel to send work
# ventilator_send = context.socket(zmq.PUB)
# ventilator_send.bind("tcp://127.0.0.1:5555")
# # Give everything a second to spin up and connect
# time.sleep(1)
# ventilator_send.send_multipart([b"ML", b"hello"])
# time.sleep(1)




socket = context.socket(zmq.SUB)

# Connects to a bound socket
socket.connect("tcp://127.0.0.1:5555")

# Subscribes to all topics
socket.subscribe("")

# Receives a string format message
# massimo = socket.recv_string() # receive just one string
# print(massimo)
massimo = socket.recv_multipart()
print(massimo)
