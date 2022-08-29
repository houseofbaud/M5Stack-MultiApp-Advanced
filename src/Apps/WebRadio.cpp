#include "WebRadio.h"

void WebRadioClass::getvolume()
{
	preferences.begin("Volume", false);
	M5m.vol = preferences.getFloat("vol", 15.0f);
	preferences.end();
}

void WebRadioClass::setVolume(int v)
{
	// float volume = *v / 71.4f; // volme max value can be 3.99
	float volume = (float)v / 200.0; // volme max value can be 3.99
	out->SetGain(volume);
}

void WebRadioClass::StopPlaying()
{
	if (player)
	{
		player->stop();
		delete player;
		player = NULL;
	}
	if (buff)
	{
		buff->close();
		delete buff;
		buff = NULL;
	}
	if (file)
	{
		file->close();
		delete file;
		file = NULL;
	}
}

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
String _s2, _s3;
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
	String s1 = string;
	String s2 = strstr(s1.c_str(), "/");
	String s3 = s1;
	s3.replace(s2, "");
	s2.replace("/", "");
	M5m.Lcd.setTextColor(TFT_BLACK);
	M5m.Lcd.setFreeFont(MDFONT);
	M5m.Lcd.drawCentreString(_s3, M5m.Lcd.width()/2, 80, 1);
	M5m.Lcd.drawCentreString(_s2, M5m.Lcd.width()/2, 105, 1);
	M5m.Lcd.setTextColor(TFT_ORANGE);
	M5m.Lcd.drawCentreString(s3, M5m.Lcd.width()/2, 80, 1);
	M5m.Lcd.drawCentreString(s2, M5m.Lcd.width()/2, 105, 1);
	_s2 = s2;
	_s3 = s3;
}

bool WebRadioClass::GetStations(fs::FS &fs, const char *path)
{
	File sdfile = fs.open(path);
	if (!sdfile)
	{
		Name.push_back("TOP Radio");
		Link.push_back("http://playerservices.streamtheworld.com/api/livestream-redirect/TOP_RADIO.mp3");
		Name.push_back("TML");
		Link.push_back("http://23603.live.streamtheworld.com/OWR_INTERNATIONAL_SC");
		Name.push_back("Q music");
		Link.push_back("http://21293.live.streamtheworld.com/QMUSIC.mp3");
		Name.push_back("Stu Bru");
		Link.push_back("http://icecast.vrtcdn.be/stubru-high.mp3");
		Name.push_back("Stu Bru UNTZ");
		Link.push_back("http://icecast.vrtcdn.be/stubru_untz-high.mp3");
		Name.push_back("MNM");
		Link.push_back("http://icecast.vrtcdn.be/mnm-high.mp3");
		Name.push_back("MNM hits");
		Link.push_back("http://icecast.vrtcdn.be/mnm_hits-high.mp3");
		Name.push_back("Klara continuo");
		Link.push_back("http://icecast.vrtcdn.be/klaracontinuo-high.mp3");
		Name.push_back("Klara");
		Link.push_back("http://icecast.vrtcdn.be/klara-high.mp3");
		Name.push_back("Radio 1");
		Link.push_back("http://icecast.vrtcdn.be/radio1-high.mp3");
		return true;
	}

	if (sdfile.available())
	{
		StationList = sdfile.readStringUntil(EOF);
		while (StationList != "\0")
		{
			int t = StationList.indexOf("=");
			name = StationList.substring(0, t);
			StationList.replace(String(name + "="), "");
			int r = StationList.indexOf(";");
			link = StationList.substring(0, r);
			StationList.replace(String(link + ";"), "");
			while (StationList[0]==10 || StationList[0]==13)
			{
				StationList = StationList.substring(1);
			}
			Name.push_back(name);
			Link.push_back(link);
		}
	}
	sdfile.close();
	return true;
}

void WebRadioClass::Run()
{
	M5m.update();
	getvolume();
	M5m.drawAppMenu(F("WebRadio"), F("Vol-"), F("Next"), F("Vol+"));
	M5m.Lcd.setTextColor(TFT_ORANGE);
	M5m.Lcd.drawCentreString("Long press 'NEXT' to Exit", M5m.Lcd.width()/2, 190, 2);
	preallocateBuffer = heap_caps_malloc(preallocateBufferSize, MALLOC_CAP_DEFAULT);
	preallocateCodec = heap_caps_malloc(preallocateCodecSize, MALLOC_CAP_DEFAULT);
	out = new AudioOutputI2S(0, 1, 32);

	if (GetStations(My_SD, "/RadioStations.txt"))
	{
		if (WiFi.isConnected())
		{

			while (play)
			{
				unsigned long now = millis();
				if (now - lastcheck >= 1000)
				{

					M5m.Lcd.setTextColor(TFT_WHITE, 15);
					SignalStrength = map(100 + WiFi.RSSI(), 5, 90, 0, 100);
					M5m.Lcd.drawRightString("WiFi: " + String(SignalStrength) + " %", 310, 5, 2);
					lastcheck = now;
				}
				if (M5m.BtnA.wasPressed() && M5m.vol > 0)
				{
					M5m.vol -= 5;
					setVolume(M5m.vol);
				}
				if (M5m.BtnC.wasPressed() && M5m.vol < 100)
				{
					M5m.vol += 5;
					setVolume(M5m.vol);
				}
				if (M5m.BtnB.wasPressed())
				{
					StopPlaying();
					if (Station < (unsigned int)(Link.size() - 1))
					{
						Station++;
					}
					else
					{
						Station = 0;
					}
					upd = true;
				}
				if (M5m.BtnB.pressedFor(1000))
				{
					play = false;
				}
				if (M5m.vol != M5m.old_vol)
				{
					M5m.Lcd.HprogressBar(80, 170, M5m.Lcd.width()-120, 15, TFT_GREEN, M5m.vol, true);
					M5m.old_vol = M5m.vol;
				}
				M5m.update();

				if (upd)
				{
					M5m.Lcd.setTextColor(TFT_RED);
					M5m.Lcd.drawString("Buffer %", 15, 148, 2);
					M5m.Lcd.HprogressBar(80, 150, M5m.Lcd.width()-120, 15, TFT_RED, 0, true); // 200
					rawFillLvl = 0;
					M5m.Lcd.setTextColor(TFT_GREEN);
					M5m.Lcd.drawString("Volume", 15, 168, 2);
					M5m.Lcd.HprogressBar(80, 170, M5m.Lcd.width()-120, 15, TFT_GREEN, M5m.vol, true); // 200
					M5m.Lcd.setFreeFont(STFONT);
					M5m.Lcd.setTextColor(TFT_BLACK);
					M5m.Lcd.drawCentreString(old_Station, M5m.Lcd.width()/2, 35, 1);
					M5m.Lcd.setTextColor(TFT_PINK);
					M5m.Lcd.drawCentreString(Name[Station], M5m.Lcd.width()/2, 35, 1);
					MDCallback(NULL, NULL, false, "/");
					old_Station = Name[Station];
					file = new AudioFileSourceICYStream(Link[Station].c_str());
					file->RegisterMetadataCB(MDCallback, (void *)"ICY");
					buff = new AudioFileSourceBuffer(file, preallocateBuffer, preallocateBufferSize);
					player = new AudioGeneratorMP3(preallocateCodec, preallocateCodecSize);
					player->begin(buff, out);
					setVolume(M5m.vol);
					M5m.old_vol = M5m.vol;
					upd = false;
				}
				else
				{
					player->loop();
					if (rawFillLvl != buff->getFillLevel())
					{
						rawFillLvl = buff->getFillLevel();
						fillLvl = map(rawFillLvl, 0, preallocateBufferSize, 0, 100);
						M5m.Lcd.HprogressBar(80, 150, M5m.Lcd.width()-120, 15, TFT_RED, fillLvl, true);
					}
					if (!upd && !fillLvl)
					{
						StopPlaying();
						if (Station < (unsigned int)(Link.size() - 1))
						{
							Station++;
							upd = true;
						}
						else
						{
							break;
						}
					}
				}
			}
			preferences.begin("Volume", false);
			preferences.putFloat("vol", M5m.vol);
			preferences.end();
			StopPlaying();
			if (out)
			{
				out->stop();
				delete out;
				out = NULL;
			}
			heap_caps_free(preallocateBuffer);
			heap_caps_free(preallocateCodec);
			dacWrite(25, 0);
			dacWrite(26, 0);
		}
		else
		{
			M5m.Lcd.setTextColor(TFT_WHITE);
			M5m.Lcd.drawCentreString("Wifi Not Connected!", M5m.Lcd.width()/2, 60, 2);
			delay(3000);
		}
	}
	else
	{
		M5m.Lcd.setTextColor(TFT_WHITE);
		M5m.Lcd.drawCentreString("RadioStations.txt Not Found on SD", M5m.Lcd.width()/2, 60, 2);
		delay(3000);
	}
}

WebRadioClass::WebRadioClass()
{
}

WebRadioClass::~WebRadioClass()
{
	Name.clear();
	Name.shrink_to_fit();
	Link.clear();
	Link.shrink_to_fit();
	M5m.Lcd.fillScreen(0);
	M5m.Lcd.setTextSize(1);
	M5m.Lcd.setTextFont(1);
	M5m.show();
}
