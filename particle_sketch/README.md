# particle_sketch

A Particle project named particle_sketch

## Welcome to your project!

### `docker` directory:

This is where we define docker containers for use in development / building firmware.

### `/src` directory:  

This is the source directory that contains the C/C++ code for building to firmware. The `.ino` file It contains a `setup()` and `loop()` function, and can be written in Wiring or C/C++. For more information about using the Particle firmware API to create firmware for your Particle device, refer to the [Firmware Reference](https://docs.particle.io/reference/firmware/) section of the Particle documentation.

### `project.properties` file:  

This is the file that specifies the name and version number of the libraries that your project depends on. Dependencies are added automatically to your `project.properties` file when you add a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

## Adding additional files to your project

### Projects with multiple sources

If you would like add additional files to your application, they should be added to the `/src` directory. All files in the `/src` directory will be sent to the Particle Cloud to produce a compiled binary.

### Projects with external libraries

If your project includes a library that has not been registered in the Particle libraries system, you should create a new directory named `/lib/<libraryname>/src` under `/<project dir>` and add the `.h` and `.cpp` files for your library there. All contents of the `/lib` directory and subdirectories will also be sent to the Cloud for compilation.

## Compiling your project

When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project directory will be sent to the compile service:

- Everything in the `/src` directory, including your `.ino` application file
- The `project.properties` file for your project
- Any libraries stored under `lib/<libraryname>/src`

## Serial commands

If the Photon is in listening mode (blinking dark blue), configuration can also be done using the USB Serial port. Each of these commands only requires that you type the command letter (case-sensitive):

i - Prints the device ID (24 character hexadecimal string)
f - Firmware update (using ymodem)
x - Exit listening mode
s - Print system_module_info
v - System firmware version
L - Safe listen mode (does not run user code concurrently with listening mode)