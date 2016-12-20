var app = require('http').createServer(handler)
var io = require('socket.io')(app);
var fs = require('fs');

app.listen(8900);

//app.use('/jquery', require(__dirname + '/node_modules/jquery/dist/'));

function handler (req, res) {
  fs.readFile(__dirname + '/public/index.html',
  function (err, data) {
    if (err) {
      res.writeHead(500);
      return res.end('Error loading index.html');
    }

    res.writeHead(200);
    res.end(data);
  });
}




io.on('connection', function (socket) {
	console.log("client connected");
  
  socket.on('pixels', function (data) {
	socket.broadcast.emit('frame', { pixels: data });
  });

  socket.on('frame', function (_xoffset, _yoffset, _pixels) {
	socket.broadcast.emit('frameData', { xoffset: _xoffset, yoffset: _yoffset, pixels: _pixels });
  	//console.log(data);
  });

  socket.on('xoffset', function (data) {
	socket.broadcast.emit('x', { x: data });
  });
  
  socket.on('yoffset', function (data) {
	socket.broadcast.emit('y', { y: data });
  });

// Send current time to all connected clients
function sendTime() {
    io.emit('time', { time: new Date().toJSON() });
}

// Send current time every 10 secs
setInterval(sendTime, 10000);


});
