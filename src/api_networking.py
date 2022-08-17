import binascii
import platform
import socket
import struct
import time

# import script that will be responsible for the networking
# methods within this api

first_line_written = False

# todo: separate script for the x plane networking part

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


class NetworkingClass:
    # From https://github.com/charlylima/
    # XPlaneUDP/blob/master/XPlaneUdp.py
    # and https://gitlab.bliesener.com/jbliesener/
    # PiDisplay/-/blob/master/XPlaneUDP.py

    def __init__(self, IP):
        self.UDP_IP = IP
        self.UDP_PORT = 49000

    def find_xp(self, wait=3.0):
        MCAST_GRP = '239.255.1.1'  # Standard multicast group
        MCAST_PORT = 49707  # (MCAST_PORT was 49000 for XPlane10)

        # Set up to listen for a multicast beacon
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        if platform.system() == 'Windows':
            sock.bind(('', MCAST_PORT))
        else:
            sock.bind((MCAST_GRP, MCAST_PORT))
        mreq = struct.pack("=4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

        if wait > 0:
            sock.settimeout(wait)

        beacon_data = {}

        try:
            packet, sender = sock.recvfrom(15000)
            header = packet[0:5]
            if header != b"BECN\x00":
                # We assume X-Plane is the only multicaster on this port
                print("Unknown packet from " + sender[0])
                print(str(len(packet)) + " bytes")
                print(packet)
                print(binascii.hexlify(packet))

            else:
                data = packet[5:21]

                (beacon_major_version, beacon_minor_version, application_host_id,
                 xplane_version_number, role, port) = struct.unpack("<BBiiIH", data)

                computer_name = packet[21:]  # Python3, these are bytes, not a string
                computer_name = computer_name.split(b'\x00')[0]  # get name upto, but excluding first null byte

                if all([beacon_major_version == 1,
                        beacon_minor_version == 2,
                        application_host_id == 1]):
                    beacon_data = {
                        'ip': sender[0],
                        'port': port,
                        'hostname': computer_name.decode('utf-8'),
                        'xplane_version': xplane_version_number,
                        'role': role
                    }

                    xplane_found = True

        except socket.timeout:
            print ("X-Plane not found")

        sock.close()
        return beacon_data

    def send_commands(self, value, semaphore, xplane_found):
        if xplane_found:
            try:
                cmd = b'DREF'

                msg = struct.pack('<4sxf500s', cmd, value, 'BMUCustom/Value'.encode('utf-8'))
                s.sendto(msg, (self.UDP_IP, self.UDP_PORT))
                msg = struct.pack('<4sxf500s', cmd, semaphore, 'BMUCustom/Semaphore'.encode('utf-8'))
                s.sendto(msg, (self.UDP_IP, self.UDP_PORT))

                print ("X-Plane commands sent successfully")
                
            except Exception as e:
                print ("X-Plane exception occurred:", e)

        else:
            print ("X-Plane not running")
