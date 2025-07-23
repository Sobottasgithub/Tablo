# Tablo

## Run TabloNode: 
```cmd
cd src/TabloNode/ 
sudo docker compose up --build
```

## Run TabloMaster:
```cmd
cd src/TabloMaster/
apt install < requirement.txt
g++ -o tabloMaster tabloMaster.cpp ui/*.cpp 
utils/*.cpp
./tabloMaster
```

## Building:

### Using nix (with flakes and nix-command enabled):

```bash
  nix build github:Sobottasgithub/Tablo
```

### Standalone:

1. clone repository

```bash
  git clone https://github.com/sobottasgithub/Tablo
```

2. configure for your system
```bash
 cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

3. compile

```bash
  cmake --build build
```

4. install for packaging
```
   cmake --install build
```

### Using the nix generated docker images

The flake in this repository allowes you to build minimal docker images that avoid as much bloat as possible.

To use this feature, you NEED to use nix. Download available at: https://nixos.org/

You can either run the builds yourself or use the `./build-image-nix.sh` script!
