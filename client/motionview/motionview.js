
var MotionView = function() {
	this.socket = null;
};

MotionView.prototype = {
	init: function(socketUrl, chartElem, orientationElem) {
		var self = this;
		
		self.socket = new WebSocket(socketUrl);
		self.socket.onopen = function(evt) { self.onSocketOpen(evt); };
		self.socket.onclose = function(evt) { self.onSocketClose(evt); };
		self.socket.onmessage = function(evt) { self.onSocketMessage(evt); };
		self.socket.onerror = function(evt) { self.onSocketError(evt); };
		
		self.startTime = new Date().getTime();
		
		self.gyroData = {
			X: [],
			Y: [],
			Z: []
		};
		self.orientationData = {
			Pitch: 0,
			Roll: 0,
			Yaw: 0
		};
		
		self.chart = new CanvasJS.Chart(chartElem, {
			title: {
				text: "Sensor data",
				fontSize: 15
			},
			axisX: {
				title: "Time (s)",
				titleFontSize: 15,
				labelFontSize: 15,
				lineThickness: 1,
				tickThickness: 1
			},
			axisY: {
				title: "Value",
				titleFontSize: 15,
				labelFontSize: 15,
				lineThickness: 1,
				tickThickness: 1,
				gridThickness: 1,
				minimum: -10,
				maximum: 10
			},
			data: [
				{
					type: "line",
					lineThickness: 1,
					color: "red",
					dataPoints: self.gyroData.X
				},
				{
					type: "line",
					lineThickness: 1,
					color: "green",
					dataPoints: self.gyroData.Y
				},
				{
					type: "line",
					lineThickness: 1,
					color: "blue",
					dataPoints: self.gyroData.Z
				}
			]
		});
		
		self.orientationElem = document.getElementById(orientationElem);
		
		self.updateInterval = window.setInterval(function() {
			self.updateChart();
			self.updateOrientation();
		}, 100);
	},
	destroy: function() {
		var self = this;
		
		if (self.socket) {
			self.socket.close();
			self.socket = null;
		}
		
		if (self.updateInterval) {
			window.clearInterval(self.updateInterval);
			self.updateInterval = null;
		}
	},
	updateChart: function() {
		var self = this;
		
		var lenGyroX = self.gyroData.X.length;
		var lenGyroY = self.gyroData.Y.length;
		var lenGyroZ = self.gyroData.Z.length;
		while (lenGyroX > 1000 || lenGyroY > 1000 || lenGyroZ > 1000) {
			if (lenGyroX > 1000) {
				self.gyroData.X.shift();
				lenGyroX--;
			}
			
			if (lenGyroY > 1000) {
				self.gyroData.Y.shift();
				lenGyroY--;
			}
			
			if (lenGyroZ > 1000) {
				self.gyroData.Z.shift();
				lenGyroZ--;
			}
		}
		
		self.chart.render();
	},
	updateOrientation: function() {
		var self = this;
		
		var $elem = $(self.orientationElem);
		$elem.empty();
		$("<div></div>").text("Pitch: " + self.orientationData.Pitch).appendTo($elem);
		$("<div></div>").text("Roll: " + self.orientationData.Roll).appendTo($elem);
		$("<div></div>").text("Yaw: " + self.orientationData.Yaw).appendTo($elem);
	},
	onSocketOpen: function(evt) {
	},
	onSocketClose: function(evt) {
	},
	onSocketMessage: function(evt) {
		var self = this;

		var timestamp = null;
		var accelValues = {
			X: 0,
			Y: 0,
			Z: 0
		};
		var gyroValues = {
			X: 0,
			Y: 0,
			Z: 0
		};
		var compassValues = {
			X: 0,
			Y: 0,
			Z: 0
		};
		var orientationValues = {
			Pitch: 0,
			Roll: 0,
			Yaw: 0
		};
		
		var sensors = evt.data.split(";");
		for (var i = 0; i < sensors.length; i++) {
			if (sensors[i].indexOf("TS") === 0) {
				var tspair = sensors[i].split(":");
				if (tspair.length == 2) {
					var tstoken = tspair[0];
					var tsvalue = Number(tspair[1]);
					if (tstoken == "TS")
						timestamp = tsvalue;
				}
				
				continue;
			}
			
			var axes = sensors[i].split(",");
			if (axes.length == 3) {
				for (var j = 0; j < axes.length; j++) {
					var pair = axes[j].split(":");
					if (pair.length == 2) {
						var axis = pair[0];
						var value = Number(pair[1]);
						if (axis == "AX")
							accelValues.X = value;
						else if (axis == "AY")
							accelValues.Y = value;
						else if (axis == "AZ")
							accelValues.Z = value;
						else if (axis == "GX")
							gyroValues.X = value;
						else if (axis == "GY")
							gyroValues.Y = value;
						else if (axis == "GZ")
							gyroValues.Z = value;
						else if (axis == "CX")
							compassValues.X = value;
						else if (axis == "CY")
							compassValues.Y = value;
						else if (axis == "CZ")
							compassValues.Z = value;
						else if (axis == "OP")
							orientationValues.Pitch = value;
						else if (axis == "OR")
							orientationValues.Roll = value;
						else if (axis == "OY")
							orientationValues.Yaw = value;
					}
				}
			}
		}
		
		if (timestamp) {
			self.gyroData.X.push({
				x: timestamp,
				y: gyroValues.X
			});
			self.gyroData.Y.push({
				x: timestamp,
				y: gyroValues.Y
			});
			self.gyroData.Z.push({
				x: timestamp,
				y: gyroValues.Z
			});
			
			self.orientationData.Pitch = orientationValues.Pitch;
			self.orientationData.Roll = orientationValues.Roll;
			self.orientationData.Yaw = orientationValues.Yaw;
		}
	},
	onSocketError: function(evt) {
		console.log("Socket error: " + evt);
	}
};
