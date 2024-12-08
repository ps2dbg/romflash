# romflash

romflash reimplementation, based on version 1.33

## Usage

```bash
dsreset -d myps2 0 7 && dsistart -d myps2 host1:/path/to/romflash.irx -param1 -param2 /path/to/datafile.rom && dsreset -d myps2 0 0
```

## Links

Relevant forum thread: https://bitbuilt.net/forums/index.php?threads/x.6503/  

## Implementation comparison

* Support for old kernels (pre-1.3.4 SDK) is missing.  

## License

MIT license; see `LICENSE` file for more information

