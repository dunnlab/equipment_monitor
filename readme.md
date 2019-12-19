# Equipment Monitor

The lab monitoring platform that lets you rest easy.

*This project is a work in progress, and is probably broken*

Custom lab equipment monitor from the [Dunn Lab](http://dunnlab.org).

This project is built around [Boron LTE](https://store.particle.io/collections/cellular/products/boron-lte) and [Particle Xenon](https://store.particle.io/products/xenon)
boards that have the following three tools for viewing the world:

- D0 is connected to a
[digital temperature probe](https://www.sparkfun.com/products/11050). The probe
is wired as follows: Red=3.3V from Photon, Black=Gnd, White= D0 with a 4.7K pullup
resistor to 3.3V.

- D1 and D2 are hardwired to the alarm relay on the equipment. The relay common
is connected to ground, the normally closed contact is connected to D1, and
the normally open contact is connected to D2.

- TX and RX are connected to a
[RS232 converter board to monitor](https://www.sparkfun.com/products/449) the
status of instruments that have serial output (like -80C freezers). VCC on the
converter board is wired to 3.3V from the Photon.

The intent is for each piece of equipment to have one xenon (usually) or boron, with borons distributed such that all xenons can connect back to the cloud.

## Getting set up

### Software

Install the [GUI Workbench](https://docs.particle.io/quickstart/workbench/#installation) (which is either atom-based or a vscode extension) or the [command-line only version](https://docs.particle.io/tutorials/developer-tools/cli/) of the development tools.

### Xenon/Boron monitor boards

#### Flashing

We have developed the firmware code targeting Particle OS/system firmware version xx.xx. Using newer or older versions may result in unstable/unpredictable performance.

- Plug the Photon into your computer via USB

- Put the Photon in [listening mode](https://docs.particle.io/guide/getting-started/modes/photon/#listening-mode)

### Server

TODO

## Running the monitor

TODO

## References

This project draws on a variety of others, including:

- http://diotlabs.daraghbyrne.me/3-working-with-sensors/DS18B20/  temperature monitor

- https://community.particle.io/t/serial-tutorial/26946 RS232 interface
