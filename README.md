# Tablo

## Deploy with NIX

### 1 Allow following ports in your firewall

The following ports are used to establish the network tablo needs:

- **4000**
- **4001**
- **4003**
- **4004**

### 2 Deploy

```cmd
nix shell
./build-image-nix.sh
tablo-master --interface YOURINTERFACE
tablo-node --interface YOURINTERFACE
tablo-client --master YOURMASTERSIP
```

## Deploy with DOCKER

### 1 Allow following ports in your firewall

The following ports are used to establish the network tablo needs:

- **4003**

### 2 Deploy

In docker swarm mode:

```cmd
./build-image-nix.sh
docker stack deploy --compose-file nix.compose.yml NAME --detach=false
tablo-client --master YOURMASTERIP
```

### Using the nix generated docker images

The flake in this repository allowes you to build minimal docker images that avoid as much bloat as possible.

To use this feature, you NEED to use nix. Download available at: https://nixos.org/

