# Containers for firmware development

## `particle-cli`

### `docker build`

``` bash
cd ~/repos/equipment_monitor/particle_firmware/docker/particle-cli
# for the latest version
docker build -t particle-cli .
# for some specific particle-cli version (e.g. 2.0.1)
docker build -t particle-cli --build-arg PARTICLE_CLI_VERSION=2.0.1 .
```

### Using the container

To start:

``` bash
docker run --rm -ti -e USERID=$UID \
           -v ~/repos/equipment_monitor/particle_firmware:/particle_firmware \
           -v ~/.particle:/root/.particle \
           particle-cli
```

- Replace `~/repos/equipment_monitor/particle_firmware` with wherever you have stored `equipment_monitor` on your machine.
- Remove the ~/.particle mount if you want to log in every time you start the container
- remember to login with `particle login`

To build:

```bash
cd /particle_firmware
particle compile -v xenon --saveTo xenon.bin
particle compile -v boron --saveTo boron.bin
```

To flash:

``` bash
particle flash somexenon xenon.bin
particle flash someboron boron.bin
```
