/*
 * XPlaneBeaconListener.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: shahada
 *
 *  Copyright (c) 2017-2018 Shahada Abubakar.
 *
 *  This file is part of libXPlane-UDP-Client.
 *
 *  This program is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public
 *  License as  published by the Free Software Foundation, either
 *  version 3 of the  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  See the GNU General Public License for more details.
 *
 */

//#include <arpa/inet.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <unistd.h>
//#include <syslog.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <windows.h>
#include <chrono>
#include <thread>

#include <iostream>
#include <thread>
#include <sstream>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "XPlaneBeaconListener.h"
#include <errno.h>

using namespace std;

// initialize statics
XPlaneBeaconListener * XPlaneBeaconListener::instance = NULL;

XPlaneBeaconListener::XPlaneBeaconListener() {

	debug = 0;
	quitFlag = false;
	isRunning = false;
	std::thread t(&XPlaneBeaconListener::runListener, this);
	t.detach();

}

XPlaneBeaconListener::~XPlaneBeaconListener() {

	quitFlag = true;
	time_t nowTime = time(NULL);

	while (isRunning && nowTime < time(NULL) - 5) {
		if (debug) {
			cerr << "waiting for XPlaneBeaconListener to stop" << endl;
		}
	}

	if (isRunning) {
		cerr << "... XPlaneBeaconListener failed to stop within 5 seconds."
				<< endl;
	}

	closesocket(sock);
	WSACleanup();
}

void XPlaneBeaconListener::runListener() {

	// https://web.cs.wpi.edu/~claypool/courses/4514-B99/samples/multicast.c
	// http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html

	struct sockaddr_in addr;
	//socklen_t addrlen;
	int addrlen;
	struct ip_mreq mreq;
	char message[1024];
	int messageLen = 1024;


	
	//std::cout << "Initialising Winsock..." << std::endl;
	WSADATA wsa;
	int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (wsaResult != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		throw runtime_error("Unable to create UDP socket");
	}
	
	/* set up socket */
	sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		perror("socket");
		exit(1);
	}
	memset((char*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(49707);
	addrlen = sizeof(addr);

	/* allow multiple sockets to use the same ADDR */
	u_int yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes)) < 0) {
		ostringstream buf;
		buf << "XPlaneBeaconListener: Reusing ADDR failed: " << strerror(errno);
		throw runtime_error(buf.str());
	}

	/* allow multiple sockets to use the same ADDR */
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*) &yes, sizeof(yes)) < 0) {
		ostringstream buf;
		buf << "XPlaneBeaconListener: Reusing PORT failed: " << strerror(errno);
		throw runtime_error(buf.str());
	}

	/* timeout after 1 second if no data received */
	int iVal = 2000;
	unsigned int sz = sizeof(iVal);
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iVal,
				   sizeof(iVal)))
	{
		ostringstream buf;
		buf << "XPlaneBeaconListener: set SO_RCVTIMEO failed: "	<< strerror(errno);
		throw runtime_error(buf.str());
	}

	int result, error;
	/* receive */
	result = ::bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	if (result == SOCKET_ERROR) {
		error = WSAGetLastError();
		std::cout << "Socket bind failed. Error: " << error << std::endl;
		exit(1);
	}


	/* subscribe multicast
	 *
	 * Under systemd on raspbian multicast is sometimes is not available
	 * even though "network-online.target" has been satisfied. This
	 * retries for up to 5 seconds before giving up.
	 */

	mreq.imr_multiaddr.s_addr = inet_addr("239.255.1.1");
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	int retry = 0;

	do {
		result = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq));
		if (result < 0) {
			retry++;
			printf("Retrying subscribe to multicast (attempt %d)", retry);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 1000ms
		}
	} while (result < 0 && retry < 5);
	if (retry < 0) {
		ostringstream buf;
		buf << "XPlaneBeaconListner: setsockopt IP_ADD_MEMBERSHIP failed: "
				<< strerror(errno);
		throw runtime_error(buf.str());
	}

	time_t lastExpiredCheck = time(NULL);

	isRunning = true;
	while (!quitFlag) {

		recv_len = recvfrom(sock, message, messageLen, 0, (sockaddr *) &addr, &addrlen);
		time_t nowTime = time(NULL);

		if (recv_len < 0) 
		{

			if (errno != EWOULDBLOCK) {
				continue;
				//ostringstream buf;
				//buf << "XPlaneBeaconListner: recvfrom returned " << recv_len << " errno is " << strerror(errno) << endl;
				//std::cout << buf.str() << " last error = " << WSAGetLastError() << std::endl;
				//break;
				//throw runtime_error(buf.str());
			};

		}
		else if (recv_len == 0) 
		{
			break;
		}
		else if (recv_len > 5 && memcmp(message, "BECN", 5) == 0) 
		{

			// parse the message

			XPlaneServer s(nowTime, message, inet_ntoa(addr.sin_addr));

			cachedServersMutex.lock();
			ostringstream key;
			key << s.host << ":" << s.name << ":" << s.receivePort;

			if (cachedServers.find(key.str()) == cachedServers.end()) {
				for (auto callback : callbacks) {
					callback(s, true);
				}
			}
			cachedServers[key.str()] = s;
			cachedServersMutex.unlock();

		};

		// Check the list of servers we have for expired servers.
		if (lastExpiredCheck < nowTime) {
			checkForExpiredServers();
			lastExpiredCheck = nowTime;
		}
	}

	// exit requested.
	isRunning = false;
}

XPlaneBeaconListener::XPlaneServer::XPlaneServer(time_t time, char * msg,
		char * _host) {

	prologue = msg;

	received = time;
	memcpy(&beaconMajorVersion, msg + 5, 1);
	memcpy(&beaconMinorVersion, msg + 6, 1);
	memcpy(&applicationHostId, msg + 7, 4);
	memcpy(&versionNumber, msg + 11, 4);
	memcpy(&role, msg + 15, 4);
	memcpy(&receivePort, msg + 19, 2);
	name = (char *) msg + 21;
	host = _host;

}

std::string XPlaneBeaconListener::XPlaneServer::toString() {

	ostringstream s;
	s << "XPlaneServer [ prologue: " << prologue << " beaconMajorVersion:"
			<< (int) beaconMajorVersion << " beaconMinorVersion:"
			<< (int) beaconMinorVersion << " applicationHostID:"
			<< applicationHostId << " versionNumber:" << versionNumber
			<< " role:" << role << " receivePort: " << receivePort << " name:"
			<< name << " host:" << host << "]" << endl;
	return s.str();
}

void XPlaneBeaconListener::get(
		std::list<XPlaneBeaconListener::XPlaneServer> & ret) {

	cachedServersMutex.lock();
	ret.clear();
	for (auto server : cachedServers) {
		ret.push_back(server.second);
	}

	cachedServersMutex.unlock();

}

void XPlaneBeaconListener::registerNotificationCallback(
		std::function<void(XPlaneServer server, bool exists)> callback) {
	callbacks.push_back(callback);
}

void XPlaneBeaconListener::checkForExpiredServers() {

	cachedServersMutex.lock();
	time_t nowTime = time(NULL);
	for (auto it = cachedServers.cbegin(); it != cachedServers.cend();) {
		// see if it has expired, i.e. we haven't received any beacons in last 30 seconds.
		if (it->second.received < nowTime - 30) {
			for (auto callback : callbacks) {
				callback(it->second, false);
			}
			cachedServers.erase(it++); // or "it = m.erase(it)" since C++11
		} else {
			++it;
		}
	}
	cachedServersMutex.unlock();

}
