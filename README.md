# romflash

romflash reimplementation, based on version 1.33

## Usage

If you want to run romflash on a device system that does not have DRP installed or set up, set up [RDB](https://github.com/ps2dbg/RDB).  
Run the following on a host system in a terminal with [dsnet](https://github.com/ps2dbg/dsnet) installed, replacing `myps2` with the IP address of the device running the `dsnetm` instance.  
```bash
dsreset -d myps2 0 7 && dsistart -d myps2 host1:/path/to/romflash.irx -param1 -param2 /path/to/datafile.rom && dsreset -d myps2 0 0
```

## Links

Relevant forum thread: https://bitbuilt.net/forums/index.php?threads/x.6503/  

## Implementation comparison

* Support for old kernels (pre-1.3.4 SDK) is missing.  

## License

MIT license; see `LICENSE` file for more information

