/*
Copyright © 2012 NaturalPoint Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */


/*

SampleClient.cpp

This program connects to a NatNet server, receives a data stream, and writes that data stream
to an ascii file.  The purpose is to illustrate using the NatNetClient class.

Usage [optional]:

SampleClient [ServerIP] [LocalIP] [OutputFilename]

[ServerIP]			IP address of the server (e.g. 192.168.0.107) ( defaults to local machine)
[OutputFilename]	Name of points file (pts) to write out.  defaults to Client-output.pts
1
*/


/*
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
Shay & Galit cc
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fstream>
#include "Expressions.h"
#include "Ratings.h"


using namespace std;

#ifdef _WIN32
#   include <conio.h>
#else
#   include <unistd.h>
#   include <termios.h>
#endif

#include <vector>
#include <NatNetTypes.h>
#include <NatNetCAPI.h>
#include <NatNetClient.h>

#ifndef _WIN32
char getch();
#endif
void _WriteHeader(FILE* fp, sDataDescriptions* pBodyDefs);
void _WriteFrame(FILE* fp, sFrameOfMocapData* data);
void _WriteFooter(FILE* fp);
void NATNET_CALLCONV ServerDiscoveredCallback(const sNatNetDiscoveredServer* pDiscoveredServer, void* pUserContext);
void NATNET_CALLCONV DataHandler(sFrameOfMocapData* data, void* pUserData);    // receives data from the server
void NATNET_CALLCONV MessageHandler(Verbosity msgType, const char* msg);      // receives NatNet error messages
void resetClient();
int ConnectClient();

static const ConnectionType kDefaultConnectionType = ConnectionType_Multicast;

NatNetClient* g_pClient = NULL;
FILE* g_outputFile;

std::vector< sNatNetDiscoveredServer > g_discoveredServers;
sNatNetClientConnectParams g_connectParams;
char g_discoveredMulticastGroupAddr[kNatNetIpv4AddrStrLenMax] = NATNET_DEFAULT_MULTICAST_ADDRESS;
int g_analogSamplesPerMocapFrame = 0;
sServerDescription g_serverDescription;

// expression instance
Expressions expression;


int main()
{

	cout << "press '1' to start analysing, and press 'Q' when the movie is over." << endl;
	cout << "Enjoy the movie! :)" << endl << endl;

	// Install logging callback
	NatNet_SetLogCallback(MessageHandler);

	// create NatNet client
	g_pClient = new NatNetClient(1);

	// set the frame callback handler
	g_pClient->SetFrameReceivedCallback(DataHandler, g_pClient);	// this function will receive data from the server

																	// If no arguments were specified on the command line,
																	// attempt to discover servers on the local network.
																	// An example of synchronous server discovery.
#if 0
	const unsigned int kDiscoveryWaitTimeMillisec = 5 * 1000; // Wait 5 seconds for responses.
	const int kMaxDescriptions = 10; // Get info for, at most, the first 10 servers to respond.
	sNatNetDiscoveredServer servers[kMaxDescriptions];
	int actualNumDescriptions = kMaxDescriptions;
	NatNet_BroadcastServerDiscovery(servers, &actualNumDescriptions);

	if (actualNumDescriptions < kMaxDescriptions)
	{
		// If this happens, more servers responded than the array was able to store.
	}
#endif

	// Do asynchronous server discovery.
	NatNetDiscoveryHandle discovery;
	NatNet_CreateAsyncServerDiscovery(&discovery, ServerDiscoveredCallback);

	while (const int c = getch())
	{
		if (c >= '1' && c <= '9')
		{
			const size_t serverIndex = c - '1';
			if (serverIndex < g_discoveredServers.size())
			{
				const sNatNetDiscoveredServer& discoveredServer = g_discoveredServers[serverIndex];

				if (discoveredServer.serverDescription.bConnectionInfoValid)
				{
					// Build the connection parameters.
#ifdef _WIN32
					_snprintf_s(
#else
					snprintf(
#endif
						g_discoveredMulticastGroupAddr, sizeof g_discoveredMulticastGroupAddr,
						"%" PRIu8 ".%" PRIu8".%" PRIu8".%" PRIu8"",
						discoveredServer.serverDescription.ConnectionMulticastAddress[0],
						discoveredServer.serverDescription.ConnectionMulticastAddress[1],
						discoveredServer.serverDescription.ConnectionMulticastAddress[2],
						discoveredServer.serverDescription.ConnectionMulticastAddress[3]
					);

					g_connectParams.connectionType = discoveredServer.serverDescription.ConnectionMulticast ? ConnectionType_Multicast : ConnectionType_Unicast;
					g_connectParams.serverCommandPort = discoveredServer.serverCommandPort;
					g_connectParams.serverDataPort = discoveredServer.serverDescription.ConnectionDataPort;
					g_connectParams.serverAddress = discoveredServer.serverAddress;
					g_connectParams.localAddress = discoveredServer.localAddress;
					g_connectParams.multicastAddress = g_discoveredMulticastGroupAddr;
				}
				else
				{
					// We're missing some info because it's a legacy server.
					// Guess the defaults and make a best effort attempt to connect.
					g_connectParams.connectionType = kDefaultConnectionType;
					g_connectParams.serverCommandPort = discoveredServer.serverCommandPort;
					g_connectParams.serverDataPort = 0;
					g_connectParams.serverAddress = discoveredServer.serverAddress;
					g_connectParams.localAddress = discoveredServer.localAddress;
					g_connectParams.multicastAddress = NULL;
				}

				break;
			}
		}
		else if (c == 'q')
		{
			return 0;
		}
	}

	NatNet_FreeAsyncServerDiscovery(discovery);

	int iResult;

	// Connect to Motive
	iResult = ConnectClient();
	if (iResult != ErrorCode_OK)
	{
		printf("Error initializing client.  See log for details.  Exiting");
		return 1;
	}
	else
	{
		printf("Client initialized and ready.\n");
	}

	// Send/receive test request
	void* response;
	int nBytes;

	iResult = g_pClient->SendMessageAndWait("TestRequest", &response, &nBytes);
	if (iResult == ErrorCode_OK)
	{
		printf("[SampleClient] Received: %s", (char*)response);
	}

	// Ready to receive marker stream!
	int c;
	bool bExit = false;
	while (c = getch())
	{
		switch (c)
		{

		case 'q':
			bExit = true;
			break;
		case 'r':
			resetClient();
			break;
		case 'p':
			sServerDescription ServerDescription;
			memset(&ServerDescription, 0, sizeof(ServerDescription));
			g_pClient->GetServerDescription(&ServerDescription);
			if (!ServerDescription.HostPresent)
			{
				printf("Unable to connect to server. Host not present. Exiting.");
				return 1;
			}
			break;
		case 's':
		{
			printf("\n\n[SampleClient] Requesting Data Descriptions...");
			sDataDescriptions* pDataDefs = NULL;
			iResult = g_pClient->GetDataDescriptionList(&pDataDefs);
			if (iResult != ErrorCode_OK || pDataDefs == NULL)
			{
				printf("[SampleClient] Unable to retrieve Data Descriptions.");
			}
			else
			{
				printf("[SampleClient] Received %d Data Descriptions:\n", pDataDefs->nDataDescriptions);
			}
		}
		break;
		case 'm':	                        // change to multicast
			g_connectParams.connectionType = ConnectionType_Multicast;
			iResult = ConnectClient();
			if (iResult == ErrorCode_OK)
				printf("Client connection type changed to Multicast.\n\n");
			else
				printf("Error changing client connection type to Multicast.\n\n");
			break;
		case 'u':	                        // change to unicast
			g_connectParams.connectionType = ConnectionType_Unicast;
			iResult = ConnectClient();
			if (iResult == ErrorCode_OK)
				printf("Client connection type changed to Unicast.\n\n");
			else
				printf("Error changing client connection type to Unicast.\n\n");
			break;
		case 'c':                          // connect
			iResult = ConnectClient();
			break;
		case 'd':                          // disconnect
										   // note: applies to unicast connections only - indicates to Motive to stop sending packets to that client endpoint
			iResult = g_pClient->SendMessageAndWait("Disconnect", &response, &nBytes);
			if (iResult == ErrorCode_OK)
				printf("[SampleClient] Disconnected");
			break;
		default:
			break;
		}
		if (bExit)
			break;
	}

	// Done - clean up server connection.
	if (g_pClient)
	{
		g_pClient->Disconnect();
		delete g_pClient;
		g_pClient = NULL;
	}

	if (g_outputFile)
	{
		_WriteFooter(g_outputFile);
		fclose(g_outputFile);
		g_outputFile = NULL;
	}

	//calculating statistics of expression precenteges
	MovieRatings ratings;
	ratings.calcStatistics(expression.getFramesCount(), expression.getSmileCount(), expression.getSurprisedCount(), expression.getScaredCount(), expression.getSadCount(), expression.getAngryCount(), expression.getNaturalCount());
	
	return ErrorCode_OK;
}

void NATNET_CALLCONV ServerDiscoveredCallback(const sNatNetDiscoveredServer* pDiscoveredServer, void* pUserContext)
{
	char serverHotkey = '.';
	if (g_discoveredServers.size() < 9)
	{
		serverHotkey = static_cast<char>('1' + g_discoveredServers.size());
	}

	const char* warning = "";

	if (pDiscoveredServer->serverDescription.bConnectionInfoValid == false)
	{
		warning = " (WARNING: Legacy server, could not autodetect settings. Auto-connect may not work reliably.)";
	}

	printf("[%c] %s %d.%d at %s%s\n",
		serverHotkey,
		pDiscoveredServer->serverDescription.szHostApp,
		pDiscoveredServer->serverDescription.HostAppVersion[0],
		pDiscoveredServer->serverDescription.HostAppVersion[1],
		pDiscoveredServer->serverAddress,
		warning);

	g_discoveredServers.push_back(*pDiscoveredServer);
}

// Establish a NatNet Client connection
int ConnectClient()
{
	// Release previous server
	g_pClient->Disconnect();

	// Init Client and connect to NatNet server
	int retCode = g_pClient->Connect(g_connectParams);
	if (retCode != ErrorCode_OK)
	{
		printf("Unable to connect to server.  Error code: %d. Exiting", retCode);
		return ErrorCode_Internal;
	}
	else
	{
		// connection succeeded

		void* pResult;
		int nBytes = 0;
		ErrorCode ret = ErrorCode_OK;

		// get mocap frame rate
		ret = g_pClient->SendMessageAndWait("FrameRate", &pResult, &nBytes);
		if (ret == ErrorCode_OK)
		{
			float fRate = *((float*)pResult);
			printf("Mocap Framerate : %3.2f\n", fRate);
		}
		else
			printf("Error getting frame rate.\n");

		// get # of analog samples per mocap frame of data
		ret = g_pClient->SendMessageAndWait("AnalogSamplesPerMocapFrame", &pResult, &nBytes);
		if (ret == ErrorCode_OK)
		{
			g_analogSamplesPerMocapFrame = *((int*)pResult);
			printf("Analog Samples Per Mocap Frame : %d\n", g_analogSamplesPerMocapFrame);
		}
		else
			printf("Error getting Analog frame rate.\n");
	}

	return ErrorCode_OK;
}

// DataHandler receives data from the server
// This function is called by NatNet when a frame of mocap data is available
void NATNET_CALLCONV DataHandler(sFrameOfMocapData* data, void* pUserData)
{
	NatNetClient* pClient = (NatNetClient*)pUserData;

	// Software latency here is defined as the span of time between:
	//   a) The reception of a complete group of 2D frames from the camera system (CameraDataReceivedTimestamp)
	// and
	//   b) The time immediately prior to the NatNet frame being transmitted over the network (TransmitTimestamp)
	//
	// This figure may appear slightly higher than the "software latency" reported in the Motive user interface,
	// because it additionally includes the time spent preparing to stream the data via NatNet.
	const uint64_t softwareLatencyHostTicks = data->TransmitTimestamp - data->CameraDataReceivedTimestamp;
	const double softwareLatencyMillisec = (softwareLatencyHostTicks * 1000) / static_cast<double>(g_serverDescription.HighResClockFrequency);

	// Transit latency is defined as the span of time between Motive transmitting the frame of data, and its reception by the client (now).
	// The SecondsSinceHostTimestamp method relies on NatNetClient's internal clock synchronization with the server using Cristian's algorithm.
	const double transitLatencyMillisec = pClient->SecondsSinceHostTimestamp(data->TransmitTimestamp) * 1000.0;

	if (g_outputFile)
	{
		_WriteFrame(g_outputFile, data);
	}

	int i = 0;


	// Only recent versions of the Motive software in combination with ethernet camera systems support system latency measurement.
	// If it's unavailable (for example, with USB camera systems, or during playback), this field will be zero.
	const bool bSystemLatencyAvailable = data->CameraMidExposureTimestamp != 0;



	// FrameOfMocapData params
	bool bIsRecording = ((data->params & 0x01) != 0);
	bool bTrackedModelsChanged = ((data->params & 0x02) != 0);
	if (bIsRecording)
		printf("RECORDING\n");
	if (bTrackedModelsChanged)
		printf("Models Changed.\n");

	//expression.setFrameCount();

	// labeled markers - this includes all markers (Active, Passive, and 'unlabeled' (markers with no asset but a PointCloud ID)
	bool bOccluded;     // marker was not visible (occluded) in this frame
	bool bPCSolved;     // reported position provided by point cloud solve
	bool bModelSolved;  // reported position provided by model solve
	bool bHasModel;     // marker has an associated asset in the data stream
	bool bUnlabeled;    // marker is 'unlabeled', but has a point cloud ID that matches Motive PointCloud ID (In Motive 3D View)
	bool bActiveMarker; // marker is an actively labeled LED marker
	
	//calling expression recogniser function
	expression.expressionsRecogniser(data->LabeledMarkers);

	for (i = 0; i < data->nLabeledMarkers; i++)
	{
		bOccluded = ((data->LabeledMarkers[i].params & 0x01) != 0);
		bPCSolved = ((data->LabeledMarkers[i].params & 0x02) != 0);
		bModelSolved = ((data->LabeledMarkers[i].params & 0x04) != 0);
		bHasModel = ((data->LabeledMarkers[i].params & 0x08) != 0);
		bUnlabeled = ((data->LabeledMarkers[i].params & 0x10) != 0);
		bActiveMarker = ((data->LabeledMarkers[i].params & 0x20) != 0);


		sMarker marker = data->LabeledMarkers[i];

		// Marker ID Scheme:
		// Active Markers:
		//   ID = ActiveID, correlates to RB ActiveLabels list
		// Passive Markers: 
		//   If Asset with Legacy Labels
		//      AssetID 	(Hi Word)
		//      MemberID	(Lo Word)
		//   Else
		//      PointCloud ID
		int modelID, markerID;
		NatNet_DecodeID(marker.ID, &modelID, &markerID);

		char szMarkerType[512];
		if (bActiveMarker)
			strcpy(szMarkerType, "Active");
		else if (bUnlabeled)
			strcpy(szMarkerType, "Unlabeled");
		else
			strcpy(szMarkerType, "Labeled");
	}
}

// MessageHandler receives NatNet error/debug messages
void NATNET_CALLCONV MessageHandler(Verbosity msgType, const char* msg)
{
	// Optional: Filter out debug messages
	if (msgType < Verbosity_Info)
	{
		return;
	}

	printf("\n[NatNetLib]");

	switch (msgType)
	{
	case Verbosity_Debug:
		printf(" [DEBUG]");
		break;
	case Verbosity_Info:
		printf("  [INFO]");
		break;
	case Verbosity_Warning:
		printf("  [WARN]");
		break;
	case Verbosity_Error:
		printf(" [ERROR]");
		break;
	default:
		printf(" [?????]");
		break;
	}

	printf(": %s\n", msg);
}


/* File writing routines */
void _WriteHeader(FILE* fp, sDataDescriptions* pBodyDefs)
{
	int i = 0;

	if (!pBodyDefs->arrDataDescriptions[0].type == Descriptor_MarkerSet)
		return;

	sMarkerSetDescription* pMS = pBodyDefs->arrDataDescriptions[0].Data.MarkerSetDescription;
	fprintf(fp, "<MarkerSet>\n\n");
	fprintf(fp, "<Name>\n%s\n</Name>\n\n", pMS->szName);
	fprintf(fp, "<Markers>\n");
	for (i = 0; i < pMS->nMarkers; i++)
	{
		fprintf(fp, "%s\n", pMS->szMarkerNames[i]);
	}
	fprintf(fp, "</Markers>\n\n");

	fprintf(fp, "<Data>\n");
	fprintf(fp, "Frame#\t");
	for (i = 0; i < pMS->nMarkers; i++)
	{
		fprintf(fp, "M%dX\tM%dY\tM%dZ\t", i, i, i);
	}
	fprintf(fp, "\n");

}


void _WriteFrame(FILE* fp, sFrameOfMocapData* data)
{
	fprintf(fp, "%d", data->iFrame);
	for (int i = 0; i < data->MocapData->nMarkers; i++)
	{
		fprintf(fp, "\t%.5f\t%.5f\t%.5f", data->MocapData->Markers[i][0], data->MocapData->Markers[i][1], data->MocapData->Markers[i][2]);
	}
	fprintf(fp, "\n");
}


void _WriteFooter(FILE* fp)
{
	fprintf(fp, "</Data>\n\n");
	fprintf(fp, "</MarkerSet>\n");
}


void resetClient()
{
	int iSuccess;

	printf("\n\nre-setting Client\n\n.");

	iSuccess = g_pClient->Disconnect();
	if (iSuccess != 0)
		printf("error un-initting Client\n");

	iSuccess = g_pClient->Connect(g_connectParams);
	if (iSuccess != 0)
		printf("error re-initting Client\n");
}


#ifndef _WIN32
char getch()
{
	char buf = 0;
	termios old = { 0 };

	fflush(stdout);

	if (tcgetattr(0, &old) < 0)
		perror("tcsetattr()");

	old.c_lflag &= ~ICANON;
	old.c_lflag &= ~ECHO;
	old.c_cc[VMIN] = 1;
	old.c_cc[VTIME] = 0;

	if (tcsetattr(0, TCSANOW, &old) < 0)
		perror("tcsetattr ICANON");

	if (read(0, &buf, 1) < 0)
		perror("read()");

	old.c_lflag |= ICANON;
	old.c_lflag |= ECHO;

	if (tcsetattr(0, TCSADRAIN, &old) < 0)
		perror("tcsetattr ~ICANON");

	//printf( "%c\n", buf );

	return buf;
}
#endif
