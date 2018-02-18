#include <stdio.h>
#include <tins/tins.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <list>

using namespace std;
using namespace Tins;

char *interface;
list<string> listssid;

void beacon_thread(int thread_idx, char *str_ssid)
{
	Dot11Beacon beacon;
	beacon.addr1(Dot11::BROADCAST);
	beacon.addr2("00:01:02:03:04:05");
	beacon.addr3(beacon.addr2());
	beacon.ssid(str_ssid);
	beacon.ds_parameter_set(8);
	beacon.supported_rates({1.0f, 2.0f, 5.5f, 11.0f, 6.0f, 9.0f, 12.0f, 18.0f});
	beacon.extended_supported_rates({4.0f, 36.0f, 48.0f, 54.0f});
	beacon.rsn_information(RSNInformation::wpa2_psk());
	
	PacketSender sender;
	RadioTap radio = RadioTap() / beacon;
	for (int i = 1; ; i++)
	{
		printf("beacon_thread:: send ssid: %s (thread[%d] count=%d)\n", str_ssid, thread_idx, i);
		try {
			sender.send(radio, interface);
		} catch (...) {
			printf("beacon_thread:: exception accured at thread[%d]\n", thread_idx);
			usleep(350000);
		}
		usleep(150000);
	}
}

bool callback(const PDU &pdu)
{
	const RadioTap &radio = pdu.rfind_pdu<RadioTap>();
	const Dot11ProbeRequest &probe_req = pdu.rfind_pdu<Dot11ProbeRequest>();

	try {
		for (list<string>::iterator it = listssid.begin(); it != listssid.end(); it++)
		{
			cout << "callback:: request: " << probe_req.addr2() << " (ssid: " << *it << ")" << endl;
			Dot11ProbeResponse probe_resp;
			probe_resp.addr1(probe_req.addr2());
			probe_resp.addr2("00:01:02:03:04:05");
			probe_resp.addr3("00:01:02:03:04:05");
			probe_resp.ds_parameter_set(11);
			probe_resp.ssid((*it).c_str());
			probe_resp.supported_rates({1.0f, 2.0f, 5.5f, 11.0f, 6.0f, 9.0f, 12.0f, 18.0f});
			probe_resp.extended_supported_rates({4.0f, 36.0f, 48.0f, 54.0f});

			RadioTap radio = RadioTap() / probe_resp;
			PacketSender sender;
			sender.send(radio, interface);
		}
	} catch (...) {
		printf("callback:: exception accured\n");
	}
	return true;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printf("Usage: %s [interface] [ssid] [[ssid2], [ssid3], ...]\n", argv[0]);
		return 0;
	}
	interface = argv[1];

	for (int i = 2; i < argc; i++)
	{
		printf("beacon_thread created for ssid(%s)\n", argv[i]);
		listssid.push_back(argv[i]);
		thread thread(beacon_thread, i - 2, argv[i]);
		thread.detach();
	}

	SnifferConfiguration config;
	config.set_promisc_mode(true);
	config.set_rfmon(true);
	Sniffer sniffer(interface, config);
	sniffer.sniff_loop(callback);
    return 0;
}