#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "Log.h"
#include "Defines.h"

namespace CLASSICDIY
{
	class OTA
	{
	public:
		OTA() {};
		void begin(AsyncWebServer* asyncServer);

	private:
        AsyncWebServer* _pAsyncServer;
	};

    const  char update_firmware_html[] PROGMEM =  R"rawliteral(
        <!DOCTYPE html><html lang=\"en\"><head><meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
        <title>{n}</title>
        </head><body>
            <div>
                <h2>{n}</h2>
                <div style='font-size: .7em;'>Firmware config version '{v}'</div>
                <hr>
            </div>
            <p>Copyright (c) 2025. ClassicDIY</br></p>
            <p>The source code is hosted and available on Github at <a href="https://github.com/classicdiy/HydroFloat" target="_blank">https://github.com/classicdiy/HydroFloat</a>.</br></p>
            <p>The software is provided on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.</br></p>

            <h3>Firmware Update</h3>
            <form method='POST' action='/doupdate' enctype='multipart/form-data'>
            <input type='file' name='firmware'>
            <input type='submit' value='Update Firmware'>
            </form>
        </body></html>
        )rawliteral";
}