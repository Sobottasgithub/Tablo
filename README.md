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

2. configure for yor system
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
