# XPlane API C++ Client

This repository contains a c++ UDP client that can be connected to through python to send commands directly to XPlane.

## Running the Setup

* Start XPlane and wait until a flight has been resumed and you are in the aircraft
* Navigate to the XPlanePackage_1.0.0/cpp_xp_server folder
	* Modify the subscriptions.txt to add or remove datarefs that you wish to subscribe to.
	*  Execute the libXplane-udp-client.exe when ready
* Navigate to the XPlanePackage_1.0.0/python_client 
	* Install python 3.7 or later and install the requirements.txt
	* Run the Main.py script to test out the API


## Python API Documentation

The Python API currently has 3 functions available to use as follows:

To receive the value of a dataref, use the getDataRef function to receive the value as a string.

```
getDataRef(self, dref):
```

To modify the value of a dataref, use the setDataRef function to alter the value. Note, value must be in string format.

```
setDataRef(self, dref, value):
```

To send a command to the cockpit, use the sendCommand, input must be in the string format.

```
sendCommand(self, dref):
```