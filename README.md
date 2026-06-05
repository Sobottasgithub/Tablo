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
sudo nix-collect-garbage -d
nix build --log-format bar-with-logs
nix run .#tablo-master -- --interface INTERFACENAME
nix run .#tablo-node -- --interface INTERFACENAME
nix run .#tablo-client -- --master MASTERIPV4
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

# Take a look at Tablos networking libs:
<b>Tablo Transfer Protocol:</b><a href="https://github.com/Sobottasgithub/TTP2">TTP2</a>

<b>Tablo UDP Discovery:</b> <a href="https://github.com/Sobottasgithub/TUD">TUD</a>
