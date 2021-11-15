#include "pch.h"
#include "CCDLC.h"
#include "mycurl.h"
#include "CMskPlugin.h"
#define _WIN32_DCOM
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Winmm.lib")
#include <comdef.h>
#include <Wbemidl.h>
#include <Mmsystem.h>
#include <WS2tcpip.h>
#include <winsock.h>
#include "Global.h"
#include "CMskRadarScreen.h"
#include "AircraftExtendedData.h"

CCDLC::CCDLC()
{
	thread CCDLC_Thread(&CCDLC::CCDLC_Parser_FN, this);
	Plugin->ThreadRunning++;
	CCDLC_Thread.detach();

	thread CCDLC_OUT_Thread(&CCDLC::CCDLC_Out_FN, this);
	Plugin->ThreadRunning++;
	CCDLC_OUT_Thread.detach();
}

void CCDLC::AddCCDLCSystemMessageToQueue(string Message)
{
	OutputCCDLCRequest r;
	r.add(Message);
}

void CCDLC::AddCCDLCSystemMessageToQueueIfTracking(string ActiveControllerId, string Message)
{
	if (ActiveControllerId == Plugin->MyId)
		AddCCDLCSystemMessageToQueue(Message);
}

void CCDLC::AddCCDLCMessageToQueue(string Callsign, string RequestType, string Request)
{
	OutputCCDLCRequest r;
	r.add(RequestType + ":" + string(Plugin->ControllerMyself().GetPositionId()) + ":" + Callsign + ":" + Request);
}

void CCDLC::AddCCDLCMessageToQueueIfTracking(string ActiveControllerId, string Callsign, string RequestType, string Request)
{
	if (Plugin->MyId == ActiveControllerId)
		AddCCDLCMessageToQueue(Callsign, RequestType, Request);
}

void CCDLC::CCDLC_Parser_FN()
{
	string last_request = "";

	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);

	char bf = '\0';
	int result = -100;

	while (!Plugin->Terminating)
	{

		sockaddr_in addrRemote = {};

		sss = socket(AF_INET, SOCK_STREAM, 0);
		if (sss == -1)
		{
			Plugin->DisplayUrgent("Failed to create socket");
			return;
		}
		addrRemote.sin_family = AF_INET;
		addrRemote.sin_port = htons(44444);

		if (inet_pton(AF_INET, "188.227.86.159", &addrRemote.sin_addr) <= 0)
			//if (inet_pton(AF_INET, "192.168.0.173", &addrRemote.sin_addr) <= 0)
		{
			Plugin->DisplayUrgent("Failed to resolve address");
			return;
		}

		DWORD timeout = 100;
		setsockopt(sss, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

		if (connect(sss, (struct sockaddr*)&addrRemote, sizeof(addrRemote)) != 0)
		{
			Plugin->__warn("CCDLC SERVER NOT AVAILABLE");
			Plugin->DisplayUrgent("Failed to connect to CCDLC server");
			this_thread::sleep_for(5000ms);
			continue;
		}
		Plugin->__warn("");

		struct sockaddr_in sin;
		int addrlen = sizeof(sin);
		if (getsockname(sss, (struct sockaddr*)&sin, &addrlen) == 0 &&
			sin.sin_family == AF_INET &&
			addrlen == sizeof(sin))
		{
			localport = ntohs(sin.sin_port);
			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(sin.sin_addr), str, INET_ADDRSTRLEN);
			localip = str;
		}

		while (!Plugin->Terminating)
		{
			// Receive first
			string cmd;
			bool error = false;

			do {
				result = recv(sss, &bf, 1, 0);
				if (result != SOCKET_ERROR && result == 1)
				{
					cmd += bf;
				}
				else if (result == SOCKET_ERROR && WSAGetLastError() != WSAETIMEDOUT)
				{
					error = true;
					break;
				}
			} while (bf != '\n');

			if (error) {
				Plugin->DisplaySilent("CCDLC connection error. Reconnecting");
				break;
			}

			rm_crlf(cmd);
			if (cmd != last_request && cmd != "")
			{
				last_request = cmd;

				//	Plugin->DisplaySilent("CCDLC: " + cmd);
				auto tokens = SplitStringByChar(cmd, ':');
				cmd = "";

				if (tokens.size() < 2)	continue;
				if (IsSystemCmd(tokens[0]))
				{
					if (tokens[0] == "EXPIRED")
					{
						Plugin->__warn("LICENSE EXPIRED");
						remove((Plugin->MyPath + "lic.dat").c_str());
						thread x([]() {
							this_thread::sleep_for(5000ms);
							exit(0);
							});
						x.detach();
					}
					continue;
				}
				{
					CCDLC_Message msg;

					msg.Author = tokens[1];
					msg.Cmd = tokens[0];
					msg.CS = tokens[2];
					msg.Id = 0;
					msg.Raw_S = cmd;

					for (size_t i = 3; i < tokens.size(); i++)
					{
						msg.Value += tokens[i] + " ";
					}
					trim(msg.Value);
					msg.Raw = tokens;
					ExecCCDLCMessage(msg);
				}
			}
			this_thread::sleep_for(10ms);
		}

	}
	send(sss, "QUIT\r\n", 7, 0);
	closesocket(sss);
	Plugin->ThreadRunning--;
}

void CCDLC::ExecCCDLCMessage(CCDLC_Message msg)
{
	if (msg.Cmd == "CHLG")
	{
		if (Plugin->Lic->check_response(msg.Raw[1], msg.Raw[2]))
		{
			std::ofstream out(Plugin->MyPath + "lic.dat");
			out << msg.Raw[1] << '\n';
			out << msg.Raw[2];
			out.close();
			Plugin->DisplayUrgent("Reg OK. Restart Euroscope");
			Plugin->__warn("PLUGIN ACTIVATED. RESTART EUROSCOPE");
		}
	}
	else if (IsRequestCmd(msg.Cmd)) 
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);

		if (p != NULL)
		{
			if (msg.Value == "REQ TRS")
			{
				InitCCDLCMessage(msg, p);
				if (p->ActualControllerInfo.TrackedByMe)
				{
					PlaySound(TEXT(string(Plugin->MyPath + "ccdlc_req.wav")).c_str(), NULL, SND_FILENAME | SND_ASYNC);
				}
			}
			else {
				if (p->CheckIfCCDLCActual(msg))
				{
					InitCCDLCMessage(msg, p);
					PlaySound(TEXT(string(Plugin->MyPath + "ccdlc.wav")).c_str(), NULL, SND_FILENAME | SND_ASYNC);
				}
			}
			Plugin->Planes->__add_computed_plane(msg.CS);
		}
	}
	else if (msg.Cmd == "FTT")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);
		if (p != NULL)
		{
			p->ActualFormularData.ExtendedData.FreeText = msg.Value;
			p->ActualFormularData.ExtendedData.ShowFreeText = p->ActualFormularData.ExtendedData.FreeText != "";
		}
	}
	else if (msg.Cmd == "TRACK")
	{
		if (msg.Raw[2] == Plugin->MyId)
			Plugin->TranslateASEL = true;
	}
	else if (msg.Cmd == "ASEL")
	{
		if (msg.Raw[1] == Plugin->ReceiveASELFrom)
		{
			Plugin->Afv->AddAFVMessageToQueue(msg.Raw[2]);
		}
	}
	else if (msg.Cmd == "MRT")
	{
		ExtendedDataRequest req;

		auto p = req.check_req(msg.CS);

		if (p != NULL)
		{
			if (msg.Raw[3] == "DCT")
			{
				p->Route.PointAfterPoint(msg.Raw[4], msg.Raw[5]);
				Plugin->ToCompute->add({ msg.CS, false, false });
			}
			else if (msg.Raw[3] == "RMP")
			{
				p->Route.DelPointByName(msg.Raw[4]);
				Plugin->ToCompute->add({ msg.CS, false, false });
			}
			else if (msg.Raw[3] == "AFT" || msg.Raw[3] == "BEF")
			{
				PointDetails exspt, newpt;
				str_to_pt(msg.Raw[5], &exspt);
				str_to_pt(msg.Raw[4], &newpt);

				auto exs_pt = p->Route.FindCoord(&exspt.Position);
				if (exs_pt.IsValid())
				{
					if (msg.Raw[3] == "AFT")
						p->Route.AddPointAfterName(newpt.Free ? "" : newpt.Name, newpt.Position, exs_pt.Name);
					else {
						p->Route.AddPointBeforeName(newpt.Free ? "" : newpt.Name, newpt.Position, exs_pt.Name);
					}

					Plugin->AllScreenCreatePermanentObject("DCT_PT" + to_string(Plugin->ManualPointIndex), newpt.Position, false, "PT" + to_string(Plugin->ManualPointIndex), "DCT", &CMskRadarScreen::OnRoutePointEvent);
					Plugin->AllScreenCreatePermanentObject("FIX_PT" + to_string(Plugin->ManualPointIndex), newpt.Position, false, "PT" + to_string(Plugin->ManualPointIndex), "FIX", &CMskRadarScreen::OnRoutePointEvent);

					Plugin->ToCompute->add({ msg.CS, false, false });
				}
			}
			else if (msg.Raw[3] == "RST")
			{
				Plugin->ToCompute->add({ msg.CS, true, true });
			}
		}
	}
	else if (msg.Cmd == "CMT")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);

		if (p != NULL && p->CCDLCRequest.IsValid())
		{
			string prevValue = p->CCDLCRequest.Text;
			if (msg.Value == "REMOVE")
				p->CCDLCRequest.Text = "";
			else p->CCDLCRequest.Text = msg.Value;

			if (p->CCDLCRequest.Text != "" && p->CCDLCRequest.Text != prevValue)
			{
				PlaySound(TEXT(string(Plugin->MyPath + "ccdlc_react.wav")).c_str(), NULL, SND_FILENAME | SND_ASYNC);
			}
		}
	}
	else if (msg.Cmd == "CLR")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);
		if (p != NULL)
			p->CCDLCRequest.Text = "";
	}
	else if (msg.Cmd == "CNL")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);
		if (p != NULL)
		{
			CancelCCDLCMessage(p);
			Plugin->Planes->__add_computed_plane(msg.CS);
		}
	}
	else if (msg.Cmd == "STS")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);

		if (p != NULL)
		{
			p->ActualFlightData.CustomData.ManualStatus = msg.Value;
			if (p->ActualFlightData.CustomData.ManualStatus == "CLEAR")	p->ActualFlightData.CustomData.ManualStatus = "";
			if (p->ActualFlightData.CustomData.ManualStatus == "READY")
			{
				p->ActualFlightData.CustomData.ReadyTime = formatTime(getCurrentTime());
			}
			else if ((p->ActualFlightData.CustomData.ManualStatus == "PUSH" || p->ActualFlightData.CustomData.ManualStatus == "START") && (p->ActualFlightData.CustomData.ActualOffBlockTime == ""))
			{
				p->ActualFlightData.CustomData.ActualOffBlockTime = formatTime(getCurrentTime());
			}
		}
	}
	else if (msg.Cmd == "LNG")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);
		if (p != NULL)
			p->ActualFlightData.CustomData.ImEnglish = msg.Value == "ENG";
	}
	else if (msg.Cmd == "RST")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);
		if (p != NULL) {
			p->ActualFlightData.CustomData.SpeedUnrestricted = msg.Value.at(0) == '1';
			p->ActualFlightData.CustomData.AltUnrestricted = msg.Value.at(1) == '1';
		}
	}
	else if (msg.Cmd == "ACDT")
	{
		try {
			string cs = msg.Raw[1];
			string owner = msg.Raw[2];
			string dct = msg.Raw[3];
			bool clr = msg.Raw[4] == "1";
			int cfl = atoi(msg.Raw[5].c_str());
			int ffl = atoi(msg.Raw[6].c_str());
			int hdg = atoi(msg.Raw[7].c_str());
			string depa_rw = msg.Raw[8];
			string dest_rw = msg.Raw[9];
			string sid = msg.Raw[10];
			string star = msg.Raw[11];
			bool ctl = msg.Raw[12] == "1";
			string lang = msg.Raw[13];
			bool spd_unrest = msg.Raw[14][0] == '1';
			bool alt_unrest = msg.Raw[14][0] == '1';
			string man = msg.Raw[15] == "CLEAR" ? "" : msg.Raw[15];
			string comm_type = msg.Raw[16];
			int spd = atoi(msg.Raw[17].c_str());
			int mch = atoi(msg.Raw[18].c_str());
			string ftx = msg.Raw[19];
			string realname = msg.Raw[20];
			int atc_rating = atoi(msg.Raw[21].c_str());
			int pilot_rating = atoi(msg.Raw[22].c_str());
			int atc_time = atoi(msg.Raw[23].c_str());
			int pilot_time = atoi(msg.Raw[24].c_str());

			auto plane = Plugin->RadarTargetSelect(cs.c_str());
			if (plane.IsValid())
			{
				auto fp = plane.GetCorrelatedFlightPlan();
				if (fp.IsValid())
				{
					if (Plugin->MyId != "" && owner == Plugin->MyId)
					{
						fp.StartTracking();
					}
					if (dct != "")
					{
						fp.GetControllerAssignedData().SetDirectToPointName(dct.c_str());
					}
					// CLearance ???
					if (cfl != 0)
					{
						fp.GetControllerAssignedData().SetClearedAltitude(cfl);
					}
					if (ffl != 0)
					{
						fp.GetControllerAssignedData().SetFinalAltitude(ffl);
					}
					if (hdg != 0)
					{
						fp.GetControllerAssignedData().SetAssignedHeading(hdg);
					}
					if (spd > 0)
					{
						fp.GetControllerAssignedData().SetAssignedSpeed(spd);
					}
					if (mch > 0)
					{
						fp.GetControllerAssignedData().SetAssignedMach(mch);
					}
					if (comm_type == "T")
					{
						fp.GetControllerAssignedData().SetCommunicationType('T');
					}
					{
						ExtendedDataRequest req;
						auto p = req.check_req(cs);
						if (p != NULL) {
							if (ctl)
							{
								p->ActualFlightData.CustomData.CTL = true;
							}
							p->ActualFlightData.CustomData.ImEnglish = lang == "ENG";
							p->ActualFlightData.CustomData.AltUnrestricted = alt_unrest;
							p->ActualFlightData.CustomData.SpeedUnrestricted = spd_unrest;
							p->ActualFlightData.CustomData.ManualStatus = man;

							p->ActualFormularData.ExtendedData.ATCRating = atc_rating;
							p->ActualFormularData.ExtendedData.FreeText = ftx;
							p->ActualFormularData.ExtendedData.Hours = pilot_time;
							p->ActualFormularData.ExtendedData.Name = realname;
							p->ActualFormularData.ExtendedData.PilotRating = pilot_rating;
							p->ActualFormularData.ExtendedData.Newbie = pilot_time > 0 && pilot_time < 100;
						}
					}
				}
			}
		}
		catch (...)
		{
			Plugin->DisplayUrgent("Exception on ACDT");
		}
	}
	else if (msg.Cmd == "APP")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);
		if (p != NULL && p->CCDLCRequest.IsValid())
		{
			if (p->CCDLCRequest.Dislikes.find("x" + msg.Author + " ") != string::npos)
			{
				replaceAll(p->CCDLCRequest.Dislikes, "x" + msg.Author + " ", "");
			}
			if (p->CCDLCRequest.Likes.find(msg.Author + " ") == string::npos)
				p->CCDLCRequest.Likes = p->CCDLCRequest.Likes + msg.Author + " ";
			PlaySound(TEXT(string(Plugin->MyPath + "ccdlc_react.wav")).c_str(), NULL, SND_FILENAME | SND_ASYNC);
		}
	}
	else if (msg.Cmd == "REJ")
	{
		ExtendedDataRequest req;
		auto p = req.check_req(msg.CS);
		if (p != NULL && p->CCDLCRequest.IsValid())
		{
			if (p->CCDLCRequest.Likes.find(msg.Author + " ") != string::npos)
			{
				replaceAll(p->CCDLCRequest.Likes, msg.Author + " ", "");
			}
			if (p->CCDLCRequest.Dislikes.find("x" + msg.Author + " ") == string::npos)
				p->CCDLCRequest.Dislikes = p->CCDLCRequest.Dislikes + "x" + msg.Author + " ";

			PlaySound(TEXT(string(Plugin->MyPath + "ccdlc_react.wav")).c_str(), NULL, SND_FILENAME | SND_ASYNC);
		}
	}
}

void CCDLC::InitCCDLCMessage(CCDLC_Message msg, AircraftExtendedData* plane)
{
	plane->CCDLCRequest.CS = msg.CS;
	plane->CCDLCRequest.Dislikes = "";
	plane->CCDLCRequest.Init = msg.Author;
	plane->CCDLCRequest.Likes = "";

	plane->CCDLCRequest.Request = msg.Cmd != "TXT" ? (msg.Cmd + " " + msg.Value) : msg.Value;
	plane->CCDLCRequest.Text = "";
}

void CCDLC::CancelCCDLCMessage(AircraftExtendedData* plane)
{
	plane->CCDLCRequest = ActiveCCDLCRequest();
}

void CCDLC::CCDLC_Out_FN()
{
	while (!Plugin->Terminating)
	{
		{
			lock_guard<mutex> lock(ccdlcLockOutput);
			while (ccdlcOutMessages.size() > 0)
			{
				auto s = ccdlcOutMessages.front();
				ccdlcOutMessages.pop();

				s = s + "\r\n";

				if (send(sss, s.c_str(), s.length(), 0) == SOCKET_ERROR)
				{
					Plugin->DisplayUrgent("Failed to send data to CCDLC server");
				}
			}

			/*
			auto crl = mycurl();
			if (ccdlcOutMessages.size() > 0)
			{
				auto s = ccdlcOutMessages.front();
				ccdlcOutMessages.pop();
				crl.get(s);
			}
			*/
		}
		this_thread::sleep_for(10ms);
	}
	Plugin->ThreadRunning--;
}

InputCCDLCRequest::InputCCDLCRequest()
{
	__lock.lock();
}

InputCCDLCRequest::~InputCCDLCRequest()
{
	__lock.unlock();
}

OutputCCDLCRequest::OutputCCDLCRequest()
{
	__lock.lock();
}

void OutputCCDLCRequest::add(string request)
{
	Plugin->Ccdlc->ccdlcOutMessages.push(request);
}

OutputCCDLCRequest::~OutputCCDLCRequest()
{
	__lock.unlock();
}
