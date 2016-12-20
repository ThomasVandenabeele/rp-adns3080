$(document).ready(function() {
  		var socket = io();
  		var frameRate = 0;
		socket.on('frame', function (data) {
    			//console.log(data);
			frameRate = frameRate + 1;
			var view   = new Int8Array(data.pixels);
			//console.log(view);
    			document.getElementById('wrapper').innerHTML = '';
			for (var i = 0; i<900; i++) {
				pixDraw(view[i]); //Math.round(view[i]));
			}
   			// socket.emit('my other event', { my: 'data' });
  		});

		setInterval(function(){ 
			console.log(frameRate);
			//var p = document.getElementById('frate');
			//p.innerHTML = 'Framerate: ' + frameRate + 'fps':
			$("#frate").text('Framerate: ' + frameRate.toString() + 'fps');
			frameRate = 0;
		}, 1000);		

  		function pixDraw(clr) {
			var pixDiv = document.createElement('div');
			pixDiv.className = "pixel";
			pixDiv.style.backgroundColor = "rgb("+ clr +","+ clr +","+clr+")";
			document.getElementById("wrapper").appendChild(pixDiv);
		}

	});