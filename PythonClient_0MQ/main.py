from XPlane_API import XPlaneClient
import time

# Specify a topic for your program to communicate through
# Specify also as input IP address of pc containing xplane if not local
client = XPlaneClient("Python1")

# Call the connect command
client.connect()

# Modify and read value at same time
for i in range(0, 100, 1):
    client.setDataRef("sim/cockpit2/engine/actuators/throttle_ratio[0]", str(i/100))
    dataRefVal = client.getDataRef("sim/cockpit2/engine/actuators/throttle_ratio[0]")
    print(f"Value received for dref = {dataRefVal}")
    time.sleep(0.005)

## Example for monitoring a dataref value
# while(True):
#     dataRefVal = client.getDataRef("sim/cockpit2/engine/actuators/throttle_ratio[0]")
#     print(f"Value received for dref = {dataRefVal}")
#     time.sleep(0.005)