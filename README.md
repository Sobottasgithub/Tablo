# Tablo

## Run TabloNode: 
cd src/TabloNode/
sudo docker compose up --build

## Run TabloMaster
cd src/TabloMaster/
apt install < requirement.txt
g++ -o tabloMaster tabloMaster.cpp ui/*.cpp utils/*.cpp
./tabloMaster