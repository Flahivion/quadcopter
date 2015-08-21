
var OrientationView = function() {
};

OrientationView.prototype = {
	init: function(canvas) {
		var self = this;
		
		var gl = canvas.getContext("webgl") || canvas.getContext("experimental-webgl");
		if (!gl) {
			console.error("Failed to initialize WebGL.");
			return false;
		}
		
		if (gl) {
			gl.clearColor(0.5, 0.5, 0.5, 1.0);
			gl.clearDepth(1.0);
			gl.enable(gl.DEPTH_TEST);
			gl.depthFunc(gl.LEQUAL);
		}
		
		self.canvas = canvas;
		self.gl = gl;
		self.mvStack = [];
		
		self.initShaders();
		self.initBuffers();
		
		return true;
	},
	initShaders: function() {
		var self = this;
		
		var fragmentShader = self.getShader("shader-fs");
		var vertexShader = self.getShader("shader-vs");
		
		if (!fragmentShader || !vertexShader) {
			console.error("Error initializing shaders. Fragment: " + fragmentShader + "; Vertex: " + vertexShader);
		}
		
		self.shaderProgram = self.gl.createProgram();
		self.gl.attachShader(self.shaderProgram, vertexShader);
		self.gl.attachShader(self.shaderProgram, fragmentShader);
		self.gl.linkProgram(self.shaderProgram);
		
		if (!self.gl.getProgramParameter(self.shaderProgram, self.gl.LINK_STATUS)) {
			console.error("Error initializing shaders.");
		}
		
		self.gl.useProgram(self.shaderProgram);
		
		self.vertexPositionAttribute = self.gl.getAttribLocation(self.shaderProgram, "aVertexPosition");
		self.gl.enableVertexAttribArray(self.vertexPositionAttribute);
		
		self.vertexColorAttribute = self.gl.getAttribLocation(self.shaderProgram, "aVertexColor");
		self.gl.enableVertexAttribArray(self.vertexColorAttribute);
	},
	initBuffers: function() {
		this.initCubeBuffer(1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
		this.initAxisBuffer();
		this.initDynamicLineBuffer();
	},
	initCubeBuffer: function(colorRed, colorGreen, colorBlue, colorAlpha, altColorRed, altColorGreen, altColorBlue, altColorAlpha) {
		var self = this;
		
		var size = 0.1;
		
		var vertices = [
			// Front face
			-size, -size,  size,
			 size, -size,  size,
			 size,  size,  size,
			-size,  size,  size,
			
			// Back face
			-size, -size, -size,
			-size,  size, -size,
			 size,  size, -size,
			 size, -size, -size,
			
			// Top face
			-size,  size, -size,
			-size,  size,  size,
			 size,  size,  size,
			 size,  size, -size,
			
			// Bottom face
			-size, -size, -size,
			 size, -size, -size,
			 size, -size,  size,
			-size, -size,  size,
			
			// Right face
			 size, -size, -size,
			 size,  size, -size,
			 size,  size,  size,
			 size, -size,  size,
			
			// Left face
			-size, -size, -size,
			-size, -size,  size,
			-size,  size,  size,
			-size,  size, -size
		];
		
		self.cubeVerticesBuffer = self.gl.createBuffer();
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.cubeVerticesBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(vertices), self.gl.STATIC_DRAW);
		
		// Primary color
		
		var colors = [
			[colorRed, colorGreen, colorBlue, colorAlpha],    // Front face
			[colorRed, colorGreen, colorBlue, colorAlpha],    // Back face
			[colorRed, colorGreen, colorBlue, colorAlpha],    // Top face
			[colorRed, colorGreen, colorBlue, colorAlpha],    // Bottom face
			[colorRed, colorGreen, colorBlue, colorAlpha],    // Right face
			[colorRed, colorGreen, colorBlue, colorAlpha]     // Left face
		];
		  
		var generatedColors = [];
		  
		for (var j = 0; j < 6; j++) {
			var c = colors[j];
			
			for (var i = 0; i < 4; i++) {
				generatedColors = generatedColors.concat(c);
			}
		}
		
		self.cubeVerticesColorBuffer = self.gl.createBuffer();
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.cubeVerticesColorBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(generatedColors), self.gl.STATIC_DRAW);

		// Secondary color
		
		var altColors = [
			[altColorRed, altColorGreen, altColorBlue,  altColorAlpha],    // Front face
			[altColorRed, altColorGreen, altColorBlue,  altColorAlpha],    // Back face
			[altColorRed, altColorGreen, altColorBlue,  altColorAlpha],    // Top face
			[altColorRed, altColorGreen, altColorBlue,  altColorAlpha],    // Bottom face
			[altColorRed, altColorGreen, altColorBlue,  altColorAlpha],    // Right face
			[altColorRed, altColorGreen, altColorBlue,  altColorAlpha]     // Left face
		];
		  
		var generatedAltColors = [];
		  
		for (var j = 0; j < 6; j++) {
			var c = altColors[j];
			
			for (var i = 0; i < 4; i++) {
				generatedAltColors = generatedAltColors.concat(c);
			}
		}
		
		self.cubeVerticesAltColorBuffer = self.gl.createBuffer();
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.cubeVerticesAltColorBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(generatedAltColors), self.gl.STATIC_DRAW);
		
		self.cubeVerticesIndexBuffer = self.gl.createBuffer();
		self.gl.bindBuffer(self.gl.ELEMENT_ARRAY_BUFFER, self.cubeVerticesIndexBuffer);
		
		var cubeVertexIndices = [
			0,  1,  2,      0,  2,  3,    // front
    		4,  5,  6,      4,  6,  7,    // back
		    8,  9,  10,     8,  10, 11,   // top
		    12, 13, 14,     12, 14, 15,   // bottom
		    16, 17, 18,     16, 18, 19,   // right
		    20, 21, 22,     20, 22, 23    // left
		];
		
		self.gl.bufferData(self.gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(cubeVertexIndices), self.gl.STATIC_DRAW);
	},
	initAxisBuffer: function() {
		var self = this;
		
		var lineLength = 100.0;
		
		var axisVerts = [
			0.0, 0.0, 0.0,
			lineLength, 0.0, 0.0, // X
			0.0, 0.0, 0.0,
			0.0, lineLength, 0.0, // Y
			0.0, 0.0, 0.0,
			0.0, 0.0, lineLength  // Z
		];
		
		self.axisVerticesBuffer = self.gl.createBuffer();
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.axisVerticesBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(axisVerts), self.gl.STATIC_DRAW);
		
		var colors = [
			1.0, 0.0, 0.0, 1.0, // X-axis line
			1.0, 0.0, 0.0, 1.0,
			0.0, 1.0, 0.0, 1.0, // Y-axis line
			0.0, 1.0, 0.0, 1.0,
			0.0, 0.0, 1.0, 1.0, // Z-axis line
			0.0, 0.0, 1.0, 1.0
		];
		
		self.axisVerticesColorBuffer = self.gl.createBuffer();
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.axisVerticesColorBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(colors), self.gl.STATIC_DRAW);
	},
	initDynamicLineBuffer: function() {
		var self = this;
		
		var axisVerts = [
			0.0, 0.0, 0.0,
			0.0, 0.0, 0.0
		];
		
		self.dynamicLineVBuffer = self.gl.createBuffer();
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.dynamicLineVBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(axisVerts), self.gl.DYNAMIC_DRAW);
		
		var colors = [
			0.0, 0.0, 0.0, 1.0,
			0.0, 0.0, 0.0, 1.0
		];
		
		self.dynamicLineCBuffer = self.gl.createBuffer();
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.dynamicLineCBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(colors), self.gl.DYNAMIC_DRAW);
	},
	getShader: function(elemId) {
		var self = this;
		
		var shader;
		var elem = document.getElementById(elemId);
		var script = $(elem).text();
		
		if (elem.type == "x-shader/x-fragment")
			shader = self.gl.createShader(self.gl.FRAGMENT_SHADER);
		else if (elem.type == "x-shader/x-vertex")
			shader = self.gl.createShader(self.gl.VERTEX_SHADER);
		else
			return null;
		
		self.gl.shaderSource(shader, script);
		self.gl.compileShader(shader);
		
		if (!self.gl.getShaderParameter(shader, self.gl.COMPILE_STATUS)) {
			console.error("Error compiling shader: " + self.gl.getShaderInfoLog(shader));
			return null;
		}
		
		return shader;
	},
	render: function(curPitch, curRoll, curYaw, desiredPitch, desiredRoll, desiredYaw, thrusts) {
		var self = this;
		
		var lookDist = 5.0;
		var cubes = [
			$V([-1.0,  1.0, 0.0, 0.0]),
			$V([ 1.0,  1.0, 0.0, 0.0]),
			$V([ 1.0, -1.0, 0.0, 0.0]),
			$V([-1.0, -1.0, 0.0, 0.0])
		];
		
		self.gl.clear(self.gl.COLOR_BUFFER_BIT | self.gl.DEPTH_BUFFER_BIT);
		self.perspectiveMatrix = makePerspective(45, 640.0/480.0, 0.1, 100.0);
		self.loadIdentity();
		self.mvMultMatrix(makeLookAt(-lookDist, lookDist, -lookDist, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0));

		self.renderAxis();
		
		var curRotation = self.calculateRotationMatrix(curPitch, curRoll, curYaw);
		var desiredRotation = self.calculateRotationMatrix(desiredPitch, desiredRoll, desiredYaw);
		
		self.renderOrientation(cubes, curRotation, false);
		//self.renderOrientation(cubes, desiredRotation, true);
		
		for (var i = 0; i < cubes.length; i++) {
			var cubeCur = curRotation.multiply(cubes[i]);
			var cubeDesired = desiredRotation.multiply(cubes[i]);

			var distX = Math.pow(cubeDesired.e(1) - cubeCur.e(1), 2);
			var distY = Math.pow(cubeDesired.e(2) - cubeCur.e(2), 2);
			var distZ = Math.pow(cubeDesired.e(3) - cubeCur.e(3), 2);
			var distance = Math.sqrt(distX + distY + distZ);
			
			if ((cubeCur.e(3) - cubeDesired.e(3)) < 0)
				distance = -distance;
			
			// Position offset
			self.renderDynamicLine(
				[cubeCur.e(1), cubeCur.e(2), cubeCur.e(3)],
				[cubeDesired.e(1), cubeDesired.e(2), cubeDesired.e(3)],
				[1.0, 0.0, 1.0, 1.0],
				[1.0, 0.0, 1.0, 1.0],
				1.0
			);
			
			// Thrust indicator
			self.renderDynamicLine(
				[cubeCur.e(1), cubeCur.e(2), cubeCur.e(3)],
				[cubeCur.e(1), cubeCur.e(2), cubeCur.e(3) + thrusts[i]],
				[1.0, 0.5, 0, 1.0],
				[1.0, 0.5, 0, 1.0],
				3.0
			);
		}
	},
	calculateRotationMatrix: function(pitch, roll, yaw) {
		var pitchRadians = pitch * Math.PI / 180.0;
		var rollRadians = roll * Math.PI / 180.0;
		var yawRadians = yaw * Math.PI / 180.0;
		
		var s1 = Math.sin(rollRadians);
		var s2 = Math.sin(pitchRadians);
		var s3 = Math.sin(yawRadians);
		var c1 = Math.cos(rollRadians);
		var c2 = Math.cos(pitchRadians);
		var c3 = Math.cos(yawRadians);
		
		var orientation = $M([
			[ c2 * c3, -c2 * s3, s2, 0.0 ],
			[ c1 * s3 + c3 * s1 * s2, c1 * c3 - s1 * s2 * s3, -c2 * s1, 0.0 ],
			[ s1 * s3 - c1 * c3 * s2, c3 * s1 + c1 * s2 * s3, c1 * c2, 0.0 ],
			[ 0.0, 0.0, 0.0, 1.0 ]
		]);
		
		return orientation;
	},
	renderAxis: function() {
		var self = this;
		
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.axisVerticesBuffer);
		self.gl.vertexAttribPointer(self.vertexPositionAttribute, 3, self.gl.FLOAT, false, 0, 0);
		
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.axisVerticesColorBuffer);
		self.gl.vertexAttribPointer(self.vertexColorAttribute, 4, self.gl.FLOAT, false, 0, 0);
		
		self.setMatrixUniforms();
		self.gl.drawArrays(self.gl.LINES, 0, 6);
	},
	renderCube: function(altColor) {
		var self = this;
		
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.cubeVerticesBuffer);
		self.gl.vertexAttribPointer(self.vertexPositionAttribute, 3, self.gl.FLOAT, false, 0, 0);
		
		if (altColor)
			self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.cubeVerticesAltColorBuffer);
		else
			self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.cubeVerticesColorBuffer);
		
		self.gl.vertexAttribPointer(self.vertexColorAttribute, 4, self.gl.FLOAT, false, 0, 0);
		
		self.gl.bindBuffer(self.gl.ELEMENT_ARRAY_BUFFER, self.cubeVerticesIndexBuffer);
		self.setMatrixUniforms();
		self.gl.drawElements(self.gl.TRIANGLES, 36, self.gl.UNSIGNED_SHORT, 0);
	},
	renderOrientation: function(cubes, orientation, altColor) {
		var self = this;

		self.mvPush();
		self.mvMultMatrix(orientation);
		
		for (var i = 0; i < cubes.length; i++) {
			self.mvPush();
			self.mvTranslate(cubes[i]);
			self.renderCube(altColor);
			self.mvPop();
		}
		
		self.mvPop();
	},
	renderDynamicLine: function(fromVec3, toVec3, fromColorVec4, toColorVec4, width) {
		var self = this;
	
		var axisVerts = [
			fromVec3[0], fromVec3[1], fromVec3[2],
			toVec3[0], toVec3[1], toVec3[2]
		];
		
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.dynamicLineVBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(axisVerts), self.gl.DYNAMIC_DRAW);
		self.gl.vertexAttribPointer(self.vertexPositionAttribute, 3, self.gl.FLOAT, false, 0, 0);
		
		var colors = [
			fromColorVec4[0], fromColorVec4[1], fromColorVec4[2], fromColorVec4[3],
			toColorVec4[0], toColorVec4[1], toColorVec4[2], toColorVec4[3]
		];
		
		self.gl.bindBuffer(self.gl.ARRAY_BUFFER, self.dynamicLineCBuffer);
		self.gl.bufferData(self.gl.ARRAY_BUFFER, new Float32Array(colors), self.gl.DYNAMIC_DRAW);
		self.gl.vertexAttribPointer(self.vertexColorAttribute, 4, self.gl.FLOAT, false, 0, 0);
		
		self.setMatrixUniforms();
		self.gl.lineWidth(width);
		self.gl.drawArrays(self.gl.LINES, 0, 2);
		self.gl.lineWidth(1.0);
	},
	loadIdentity: function() {
		this.mvMatrix = Matrix.I(4);
	},
	mvMultMatrix: function(m) {
		this.mvMatrix = this.mvMatrix.x(m);
	},
	mvTranslate: function(v) {
		this.mvMultMatrix(Matrix.Translation($V([v.e(1), v.e(2), v.e(3)])).ensure4x4());
	},
	mvRotate: function(angle, v) {
		var inRadians = angle * Math.PI / 180.0;
		
		var m = Matrix.Rotation(inRadians, $V([v[0], v[1], v[2]])).ensure4x4();
		this.mvMultMatrix(m);
	},
	mvPush: function() {
		this.mvStack.push(this.mvMatrix.dup());
	},
	mvPop: function() {
		this.mvMatrix = this.mvStack.pop();
	},
	setMatrixUniforms: function() {
		var self = this;
		
		var pUniform = self.gl.getUniformLocation(self.shaderProgram, "uPMatrix");
		self.gl.uniformMatrix4fv(pUniform, false, new Float32Array(self.perspectiveMatrix.flatten()));
		
		var mvUniform = self.gl.getUniformLocation(self.shaderProgram, "uMVMatrix");
		self.gl.uniformMatrix4fv(mvUniform, false, new Float32Array(self.mvMatrix.flatten()));
	}
};
