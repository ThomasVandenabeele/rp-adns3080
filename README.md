# ADNS 3080 sensor Project

*Embedded Systems Project*
Ling Zhang, Zhejiang University, Hangzhou, China
Thomas Vandenabeele, Hasselt University, Diepenbeek, Belgium

**Dependencies:**
	- Raspberry Pi
	- bcm2835 library for SPI
	- C++
	- Boost C++ Libraries
	- Node.js
	- socket.io
	- OpenCV 3

## Usage

Install all required dependencies.

### Node.js + socket.io

Run server with command in `nodejs-websocket` directory:

```
node app.js
```


### C++ Client

Run program with root access for bcm2835 library in `cpp-client` directory:

```
make
sudo ./main
```
