# Equipment Monitor

The lab monitoring platform that lets you rest easy.

*This project is a work in progress, and is probably broken*

Custom lab equipment monitor from the [Dunn Lab](http://dunnlab.org).

This project is built around
[Boron LTE](https://store.particle.io/collections/cellular/products/boron-lte)
and [Particle Xenon](https://store.particle.io/products/xenon) boards.

## Normal operation

![Image of Monitor](monitor.gif)



During normal operation, the monitors have:

- A breathing cyan light on the particle Boron/ Xenon board on the left side
of the unit. Any other color may indicate a problem connecting to the Internet.

- A blinking green light on the circuit board.

- A screen with black background and white text. White background and black text
indicates a problem, which could be an equipment alarm, elevated temperature in
the room, sensor malfunction, or other issue. See the data on the screen for
more details.

If there appears to be a problem with the monitor itself rather than the
equipment, press the restart button. This may resolve the issue.
