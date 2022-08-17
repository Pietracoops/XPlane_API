import time
from api_networking import NetworkingClass
from API_logger import logger

class XPlane_API:
    def __init__(self, IP):
        self.network = NetworkingClass(IP)
        for i in range(1,3):
            beacon = self.network.find_xp()
            self.xplane_found = (len(beacon) > 0)

            if not self.xplane_found:
                logger.info(f'XPlane was not found, trying again in 5 seconds...')
                time.sleep(5)
            else:
                break

    def view_task(self, id_):
        print ("view_task({})".format(id_))
        self.network.send_commands(id_, 1, self.xplane_found)

    def view_action(self, id_):
        print ("view_action({})".format(id_))
        self.network.send_commands(id_, 2, self.xplane_found)

    def view_constraint(self, id_):
        print ("view_constraint({})".format(id_))
        self.network.send_commands(id_, 3, self.xplane_found)

    def execute_task(self, task_id_):
        print ("execute_task({})".format(task_id_))
        self.network.send_commands(task_id_, 4, self.xplane_found)

    def execute_sequentially(self, start_task_id_):
        print ("execute_sequentially({})".format(start_task_id_))
        self.network.send_commands(start_task_id_, 5, self.xplane_found)

    def play_flight(self):
        print ("play flight")
        self.network.send_commands(0, 13, self.xplane_found)

    def pause_flight(self):
        print ("pause flight")
        self.network.send_commands(0, 14, self.xplane_found)

    def stop_flight(self):
        print ("stop flight")
        self.network.send_commands(0, 15, self.xplane_found)

    def rewind_flight(self):
        print ("rewind flight")
        self.network.send_commands(0, 16, self.xplane_found)

    def fast_forward_flight(self):
        print ("fast forward flight")
        self.network.send_commands(0, 17, self.xplane_found)

    def previous_timestep_flight(self):
        print ("jumping to previous timestep")
        self.network.send_commands(0, 18, self.xplane_found)

    def next_timestep_flight(self):
        print ("jumping to next timestep")
        self.network.send_commands(0, 19, self.xplane_found)


