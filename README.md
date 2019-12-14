# SteamworksConnectionLinux

Linux implementation of SteamworksConnection

# Commands
## Installation
```./install.sh```
## Uninstallation
```./uninstall.sh```
## Building
```./build.sh```
Uses g++ to compile source files
## Generating h/cc files
```./makeprotos.sh```

## Running
```./swc```

# Dependencies
Google Protocol Buffers v2.5.0
g++

# FIFO Interface

Use fifo `/tmp/swcpipei` to send sharecodes in `CSGO-xxxxx-xxxxx-xxxxx-xxxxx-xxxxx` format.
Use fifo `/tmp/swcpipeo` to read response. Possible responses are:

`--demo SHARECODE_TOO_OLD`

`--demo UNKNOWN_ERROR`

`--demo <demo_download_link>|<match_timestamp>`

Please note that communication has to take place 1-by-1, always wait for a response after you've sent a sharecode, otherwise the application will crash. 

This is because fifo communication requires both participants to be reading/writing, simulatenously.

For more information, read [here](http://man7.org/linux/man-pages/man7/fifo.7.html)