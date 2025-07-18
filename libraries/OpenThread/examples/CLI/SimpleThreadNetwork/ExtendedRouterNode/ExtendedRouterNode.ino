// Copyright 2025 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "OThreadCLI.h"
#include "OThreadCLI_Util.h"

// Leader node shall use the same Network Key and channel
#define CLI_NETWORK_KEY    "00112233445566778899aabbccddeeff"
#define CLI_NETWORK_CHANEL "24"
bool otStatus = true;

void setup() {
  Serial.begin(115200);
  OThread.begin(false);  // No AutoStart - fresh start
  OThreadCLI.begin();
  Serial.println("Setting up OpenThread Node as Router/Child");
  Serial.println("Make sure the Leader Node is already running");

  otStatus &= otExecCommand("dataset", "clear");
  otStatus &= otExecCommand("dataset networkkey", CLI_NETWORK_KEY);
  otStatus &= otExecCommand("dataset channel", CLI_NETWORK_CHANEL);
  otStatus &= otExecCommand("dataset", "commit active");
  otStatus &= otExecCommand("ifconfig", "up");
  otStatus &= otExecCommand("thread", "start");

  if (!otStatus) {
    Serial.println("\r\n\t===> Failed starting Thread Network!");
    return;
  }
  // wait for the node to enter in the router state
  uint32_t timeout = millis() + 90000;  // waits 90 seconds to
  while (OThread.otGetDeviceRole() != OT_ROLE_CHILD && OThread.otGetDeviceRole() != OT_ROLE_ROUTER) {
    Serial.print(".");
    if (millis() > timeout) {
      Serial.println("\r\n\t===> Timeout! Failed.");
      otStatus = false;
      break;
    }
    delay(500);
  }

  if (otStatus) {
    // print the PanID using 2 methods

    // CLI
    char resp[256];
    if (otGetRespCmd("panid", resp)) {
      Serial.printf("\r\nPanID[using CLI]: %s\r\n", resp);
    } else {
      Serial.printf("\r\nPanID[using CLI]: FAILED!\r\n");
    }

    // OpenThread API
    Serial.printf("PanID[using OT API]: 0x%x\r\n", (uint16_t)otLinkGetPanId(esp_openthread_get_instance()));
  }
  Serial.println("\r\n");
}

void loop() {
  if (otStatus) {
    Serial.println("Thread NetworkInformation: ");
    Serial.println("---------------------------");
    OThread.otPrintNetworkInformation(Serial);
    Serial.println("---------------------------");
  } else {
    Serial.println("Some OpenThread operation has failed...");
  }
  delay(10000);
}
