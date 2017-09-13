# PitchTempoPlayer by Bernard Victor Delvaux

modified to run on Raspberry Pi, headless with some keys and buttons to control


# How to install on Raspberry Pi

## Preparing libraries
```
sudo apt-get install libgtk-3-dev
sudo apt-get install libgstreamer1.0-dev
sudo apt-get install libjack-jackd2-dev
sudo apt-get install gawk
```

## Build & run
```
cd pitchtempoplayer-1.2.0/
./configure
make
sudo make install

ptplayer
```

## Edit - build cycle
```
make
sudo make install
```
