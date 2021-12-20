# Argon setup

Normally either a simple `particle setup` via the cli or the "add new device" in the particle phone app _should_ work, but I wasn't able to get it to. Give it a try, but if that doesn't work, below worked for the Argons we got to replace the Xenons.

## What you need

- Argon dev board
- Particle login credentials
- Dunn lab netid credentials (netid `s_dunnalb`)
- USB 2 one end that can connect to computere, other end micro-B cable to connect to the Argon
- Android or iPhone that can install the particle app

## Getting Started

1. Download / install [particle cli](https://docs.particle.io/tutorials/developer-tools/cli/)
    1. Log in with `particle login`
1. Install the particle phone app
1. Clone this repo, compile firmware

``` bash
git clone https://github.com/dunnlab/equipment_monitor 
cd equipment_monitor/particle_sketch
particle compile argon
```

## For each board

1. Plug in argon via USB

1. Get the Argon's MAC Address, register it on Yale's WiFi at https://regvm3.its.yale.edu/login/aup

    ``` bash
    # to get mac address
    particle serial mac 
    ```

1. Try to add it as a new device via the phone app (this adds the device to your account but fails when actually setting up the board)

1. Update & install the argon to the latest OS

    ``` bash
    # update device to latest OS
    particle usb dfu
    particle update

    # flash firmware to device
    particle usb dfu
    particle flash --usb .\argon_firmware_*.bin
    ```

1. connect to "yale wireless" with phone app (by trying the setup process again) or

    ``` bash
    particle serial wifi
    ```

## Alerts Setup

1. Log in to https://console.particle.io
    1. Verify that the device(s) you added are listed in [devices](https://console.particle.io/devices)
1. Go to Integrations and edit (if replacing) or set up a new Webhook.
1. Set the following
    - "Event Name" : `ALARM_EQIP`
    - "URL" : `https://hooks.slack.com/services/...` (replace ... with the url for the Slack app. either get from an existing webhook config or by checking the apps in the [Dunn Lab Slack](https://dunnlab.slack.com/apps))
    - "Request Type" : `POST`
    - "Request Format" : `JSON`
    - "Device" : the device you want this alert to originate from
    - "JSON Data" : Custom, paste the following, changing the `"text"` accordingly

    ``` json
    {
    "event": "{{{PARTICLE_EVENT_NAME}}}",
    "published_at": "{{{PARTICLE_PUBLISHED_AT}}}",
    "coreid": "{{{PARTICLE_DEVICE_ID}}}",
    "text": "Gene (Incubator in OML 320) says, {{{PARTICLE_EVENT_VALUE}}}"
    }
    ```