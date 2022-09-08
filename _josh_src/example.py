
# Import the API as such
from xplane_api import XPlane_API

# Declare the API object specifying the Xplane IP address
api_obj = XPlane_API("192.168.2.44")

# Check if XPlane was found, if not exit
if not api_obj.xplane_found:
    print(f'XPlane not found, we should probably terminate')
else:
    print(f'XPlane found! Continuing')


# We can then call API commands as follows:
#api_obj.view_task(5) # View an action

# api_obj.view_action(5) # View an action
#
# api_obj.view_constraint(5) # View constraint
#
api_obj.execute_task(68) # Execute a task
#
# api_obj.execute_sequentially(5) # Execute sequentially
#
# # Some other functions in Josh' API
# api_obj.rewind_flight()
#
# api_obj.play_flight()
#
# api_obj.previous_timestep_flight()
#
# api_obj.next_timestep_flight()
