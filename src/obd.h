/*
 * This program is free software; you can use it, redistribute it
 * and / or modify it under the terms of the GNU General Public License
 * (GPL) as published by the Free Software Foundation; either version 3
 * of the License or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, in a file called gpl.txt or license.txt.
 * If not, write to the Free Software Foundation Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307 USA
 */
#pragma once
#include <BluetoothSerial.h>
#include <bitset>
#include <OBDStates.h>

#include "ELMduino.h"

// ELM327
// https://cdn.sparkfun.com/assets/learn_tutorials/8/3/ELM327DS.pdf

// set default adapter name
#ifndef OBD_ADP_NAME
#define OBD_ADP_NAME        "OBDII"
#endif

#define BT_DISCOVER_TIME    10000

// https://stackoverflow.com/questions/17170646/what-is-the-best-way-to-get-fuel-consumption-mpg-using-obd2-parameters
#define AF_RATIO_GAS        17.2
#define AF_RATIO_GASOLINE   14.7
#define AF_RATIO_PROPANE    15.5
#define AF_RATIO_ETHANOL    9.0
#define AF_RATIO_METHANOL   6.4
#define AF_RATIO_HYDROGEN   34.0
#define AF_RATIO_DIESEL     14.6

// https://kraftstoff-info.de/vergleich-der-kraftstoffe-nach-energiegehalt-und-dichte
#define DENSITY_GAS         540.0
#define DENSITY_GASOLINE    740.0
#define DENSITY_PROPANE     505.0
#define DENSITY_ETHANOL     789.0
#define DENSITY_METHANOL    792.0
#define DENSITY_HYDROGEN    70.0
#define DENSITY_DIESEL      830.0

// https://en.m.wikipedia.org/w/index.php?title=OBD-II_PIDs
// https://www.goingelectric.de/wiki/Liste-der-OBD2-Codes/
#define FUEL_TYPE_GASOLINE  1
#define FUEL_TYPE_METHANOL  2
#define FUEL_TYPE_ETHANOL   3
#define FUEL_TYPE_DIESEL    4
#define FUEL_TYPE_LPG       5
#define FUEL_TYPE_CNG       6
#define FUEL_TYPE_PROPANE   7
#define FUEL_TYPE_ELECTRIC  8

#define KPH_TO_MPH          1.60934f
#define LITER_TO_GALLON     3.7854f

typedef enum {
    METRIC,
    IMPERIAL
} measurementSystem;

class OBDClass : public OBDStates {
    BluetoothSerial serialBt;
    ELM327 elm327;

    bool initDone = false;
    bool stopConnect = false;

    String devName;
    String devMac;
    char protocol;
    bool checkPidSupport = false;

    measurementSystem system = METRIC;

    std::string connectedBTAddress;
    std::string VIN;

    std::function<void(BTScanResults *scanResult)> devDiscoveredCallback = nullptr;

    std::function<char *(int)> toBitStr = [](const int value) {
        char str[33];
        snprintf(str, sizeof(str), "%s", std::bitset<32>(value).to_string().c_str());
        return strdup(str);
    };
    std::function<char *(float)> toMiles = [&](const float value) {
        char str[16];
        snprintf(str, sizeof(str), "%4.2f", system == METRIC ? value : value / KPH_TO_MPH);
        return strdup(str);
    };
    std::function<char *(int)> toMilesInt = [&](const int value) {
        char str[16];
        snprintf(str, sizeof(str), "%d", system == METRIC ? value : static_cast<int>(value / KPH_TO_MPH));
        return strdup(str);
    };
    std::function<char *(float)> toGallons = [&](const float value) {
        char str[16];
        snprintf(str, sizeof(str), "%4.2f", system == METRIC ? value : value / LITER_TO_GALLON);
        return strdup(str);
    };
    std::function<char *(float)> toMPG = [&](const float value) {
        char str[16];
        snprintf(str, sizeof(str), "%4.2f",
                 system == METRIC ? value : value == 0.0f ? 0.0f : 235.214583333333f / value);
        return strdup(str);
    };

    BTScanResults *discoverBtDevices();

    static void BTEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

public:
    OBDClass();

    void initStates();

    void begin(const String &devName, const String &devMac, char protocol = AUTOMATIC, bool checkPidSupport = false,
               measurementSystem system = METRIC);

    void end();

    void connect(bool reconnect = false);

    void loop();

    void onDevicesDiscovered(const std::function<void(BTScanResults *scanResult)> &callable);

    std::string vin() const;

    std::string getConnectedBTAddress() const;
};

extern OBDClass OBD;
