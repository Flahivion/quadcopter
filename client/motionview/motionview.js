
var MotionView = function() {
	this.socket = null;
};

MotionView.prototype = {
	init: function(socketUrl, chartElemId, orientationElemId) {
		var self = this;
		
		self.accelData = { X: [], Y: [], Z: [] };
		self.gyroData = { X: [], Y: [], Z: [] };
		self.compassData = { X: [], Y: [], Z: [] };
		self.curOrientationData = { Pitch: 0, Roll: 0, Yaw: 0 };
		self.desiredOrientationData = { Pitch: 0, Roll: 0, Yaw: 0 };
		self.curThrusts = { Rotor1: 1.0, Rotor2: 1.0, Rotor3: 1.0, Rotor4: 1.0 };
		
		self.initSocket(socketUrl);
		self.initChart(chartElemId);
		self.initOrientation(orientationElemId);
		
		self.updateInterval = window.setInterval(function() {
			self.updateChart();
			self.updateOrientation();
		}, 100);

		self.sampleCount = 0;
		self.sampleReportInterval = window.setInterval(function() {
			console.log("Samples per second: " + self.sampleCount);
			self.sampleCount = 0;
		}, 1000);
	},
	initSocket: function(socketUrl) {
		var self = this;
		
		self.socket = new WebSocket(socketUrl);
		self.socket.onopen = function(evt) { self.onSocketOpen(evt); };
		self.socket.onclose = function(evt) { self.onSocketClose(evt); };
		self.socket.onmessage = function(evt) { self.onSocketMessage(evt); };
		self.socket.onerror = function(evt) { self.onSocketError(evt); };
	},
	initChart: function(chartElemId) {
		var self = this;
		
		self.chart = new CanvasJS.Chart(chartElemId, {
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
	},
	initOrientation: function(orientationElemId) {
		var self = this;
		
		self.orientation = new OrientationView();
		self.orientation.init(document.getElementById(orientationElemId));
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

		if (self.sampleReportInterval) {
			window.clearInterval(self.sampleReportInterval);
			self.sampleReportInterval = null;
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
		
		self.orientation.render(
			self.curOrientationData.Pitch, self.curOrientationData.Roll, self.curOrientationData.Yaw,
			self.desiredOrientationData.Pitch, self.desiredOrientationData.Roll, self.desiredOrientationData.Yaw,
			[self.curThrusts.Rotor1, self.curThrusts.Rotor2, self.curThrusts.Rotor3, self.curThrusts.Rotor4]
		);
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
		var curOrientationValues = {
			Pitch: 0,
			Roll: 0,
			Yaw: 0
		};
		var desiredOrientationValues = {
			Pitch: 0,
			Roll: 0,
			Yaw: 0
		};
		var thrustValues = {
			Rotor1: 0,
			Rotor2: 0,
			Rotor3: 0,
			Rotor4: 0
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
							curOrientationValues.Pitch = value;
						else if (axis == "OR")
							curOrientationValues.Roll = value;
						else if (axis == "OY")
							curOrientationValues.Yaw = value;
						else if (axis == "DP")
							desiredOrientationValues.Pitch = value;
						else if (axis == "DR")
							desiredOrientationValues.Roll = value;
						else if (axis == "DY")
							desiredOrientationValues.Yaw = value;
						else if (axis == "R1")
							thrustValues.Rotor1 = value;
						else if (axis == "R2")
							thrustValues.Rotor2 = value;
						else if (axis == "R3")
							thrustValues.Rotor3 = value;
						else if (axis == "R4")
							thrustValues.Rotor4 = value;
					}
				}
			}
		}
		
		if (timestamp) {
			self.gyroData.X.push({ x: timestamp, y: gyroValues.X });
			self.gyroData.Y.push({ x: timestamp, y: gyroValues.Y });
			self.gyroData.Z.push({ x: timestamp, y: gyroValues.Z });
			
			self.curOrientationData.Pitch = curOrientationValues.Pitch;
			self.curOrientationData.Roll = curOrientationValues.Roll;
			self.curOrientationData.Yaw = curOrientationValues.Yaw;
			
			self.desiredOrientationData.Pitch = desiredOrientationValues.Pitch;
			self.desiredOrientationData.Roll = desiredOrientationValues.Roll;
			self.desiredOrientationData.Yaw = desiredOrientationValues.Yaw;
			
			//console.log("Cur orientation: Pitch: " + self.curOrientationData.Pitch + ", Roll: " + self.curOrientationData.Roll + ", Yaw: " + self.curOrientationData.Yaw);
			
			self.curThrusts.Rotor1 = thrustValues.Rotor1;
			self.curThrusts.Rotor2 = thrustValues.Rotor2;
			self.curThrusts.Rotor3 = thrustValues.Rotor3;
			self.curThrusts.Rotor4 = thrustValues.Rotor4;

			self.sampleCount++;
		}
	},
	onSocketError: function(evt) {
		console.log("Socket error: " + evt);
	}
};
