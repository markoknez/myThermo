Math.Tau = 2 * Math.PI;

function arcPicker(element, settings, $scope) {
	var self = this;

	self.min = 15;
	self.max = 30;

	var isPicker = false;
	var tweenDuration = 500;
	var width = 200;
	var radius = 50;
	var arcWidth = 30;
	var tickNumber = 15;
	var showValue = true;
	var bigTickFontSize = 24;
	var smallTickFontSize = 12;

	if (undefined !== settings) {
		if (undefined !== settings.isPicker) isPicker = settings.isPicker;
		if (undefined !== settings.tickNumber) tickNumber = settings.tickNumber;
		if (undefined !== settings.showValue) showValue = settings.showValue;
	}
	var tickSize = arcWidth + 5;

	var width = $(element).width();
	var height = width;
	var radius = width / 2 - arcWidth - bigTickFontSize * 2;
	// var radius = width / 4;
	tickData = d3.range(self.min, self.max, 1);	
	self.progress = 0;

	self.arc = d3.svg.arc()
		.innerRadius(radius)
		.outerRadius(radius + arcWidth);

	self.container = d3.select(element).append('svg');

	self.svg = self.container
		.attr('width', width)
		.attr('height', height)
		.append('g')
		.attr('transform', 'translate(' + width / 2 + ',' + height / 2 + ')');

	self.background = self.svg.append('path')
		.attr('class', 'arc-background')
		.datum({
			startAngle: 0,
			endAngle: Math.Tau
		})
		.attr('d', self.arc);

	self.foreground = self.svg.append('path')
		.attr('class', 'arc-foreground')
		.datum({
			startAngle: 0,
			endAngle: Math.Tau * self.progress
		})
		.attr('d', self.arc);

	self.pickerPart = self.svg.append('path')
		.attr('class', 'arc-selector')
		.datum({
			startAngle: 0,
			endAngle: 0
		})
		.attr('visibility', 'hidden')
		.attr('d', self.arc);

	if (showValue == true) {
		self.textUpper = self.container.append('text')
			.attr('class', 'arc-inner-text')
			.attr('text-anchor', 'middle')
			.attr('alignment-baseline', 'middle')
			.attr('dx', '50%')
			.attr('dy', '50%');
	}
	addTicks();


	function addTicks() {
		self.ticks = self.svg.selectAll('.arc-tick-line')
			.data(tickData)
			.enter()
			.append('line')
			.attr('class', 'arc-tick-line')
			.attr('x1', function(d) {
				return Math.sin(Math.Tau * convertValueToProgress(d) - Math.Tau / 2) * (radius + arcWidth / 2 - tickSize / 2);
			})
			.attr('x2', function(d) {
				return Math.sin(Math.Tau * convertValueToProgress(d) - Math.Tau / 2) * (radius + arcWidth / 2 + tickSize / 2);
			})
			.attr('y1', function(d) {
				return Math.cos(Math.Tau * convertValueToProgress(d) - Math.Tau / 2) * (radius + arcWidth / 2 - tickSize / 2);
			})
			.attr('y2', function(d) {
				return Math.cos(Math.Tau * convertValueToProgress(d) - Math.Tau / 2) * (radius + arcWidth / 2 + tickSize / 2);
			});
		self.tickText = self.svg.selectAll('.arc-tick-text')
			.data(tickData);
		
		self.tickXFunction = function(d, v){
			return -Math.sin(Math.Tau * convertValueToProgress(d) - Math.Tau / 2) * (radius + arcWidth + v);
		};

		self.tickYFunction = function(d, v){
			return Math.cos(Math.Tau * convertValueToProgress(d) - Math.Tau / 2) * (radius + arcWidth + v);
		};

		self.tickText.enter()
			.append('text')
			.attr('class', 'arc-tick-text')
			.attr('text-anchor', 'middle')
			.attr('alignment-baseline', 'middle');
			// .attr('dx', function(d, i) {
			// 	return self.tickXFunction(convertValueToProgress(d), 12);
			// })
			// .attr('dy', function(d, i) {
			// 	return self.tickYFunction(convertValueToProgress(d), 12);
			// })
			// .text(function(d, i) {
			// 	return d+'c';
			// });
	}

	function addTouchHandlers() {
		self.container.on('mousedown', function() {
			self.mousedown = true;
			self.refreshPicker(self.getEndAngleFromMouse());
		});
		self.container.on('mouseup', function() {
			self.mousedown = false;
		});
		self.container.on('mouseenter', function() {
			self.pickerPart.attr('visibility', 'visible');
		});
		self.container.on('mouseleave', function() {
			self.pickerPart.attr('visibility', 'hidden');
		});
		self.container.on('mousemove', function() {
			var newProgress = convertValueToProgress(Math.round(convertProgressToValue(self.getEndAngleFromMouse())));
			self.pickerPart.datum({
				startAngle: Math.Tau * newProgress - 0.01,
				endAngle: Math.Tau * newProgress + 0.01
			}).attr('d', self.arc);

			if (!self.mousedown) return;
			self.refreshPicker(newProgress);
		});
	}

	if (isPicker == true) addTouchHandlers();

	function convertProgressToValue(angle) {
		return self.min + (self.max - self.min) * angle;
	};

	function convertValueToProgress(value) {
		return (value - self.min) / (self.max - self.min);
	};

	self.setValue = function(value) {
		var newProgress = convertValueToProgress(Math.round(value));
		self.refreshPicker(newProgress);
	};

	self.getEndAngleFromMouse = function() {
		var pos = d3.mouse(self.foreground.node());
		var angle = (360 / Math.Tau * Math.atan2(pos[1], pos[0])) + 360 + 90;
		return (angle % 360) / 360;
	};

	self.refreshPicker = function(newProgress) {
		var newValue = Math.round(convertProgressToValue(newProgress));
		var newEndValue = convertValueToProgress(newValue);
		if ($scope && newEndValue != self.progress)			
			$scope.$emit('valueChanged', newValue);
		
		self.progress = newEndValue;

		self.foreground
			.transition()
			.duration(tweenDuration)
			.call(self.arcTween, 0, Math.Tau * self.progress);

		if(showValue == true){
			self.textUpper.html(function() {
				return newValue + 'c';
			});
		}

		self.tickText
			.transition()
			.text(function(d){
				if(newValue == d) return '['+d+']';
				return d;
			})
			.attr('font-size',function(d){
				if(newValue == d) return bigTickFontSize + 'px';
				return smallTickFontSize + 'px';
			})
			.attr('dx', function(d, i) {
				if(newValue == d)return self.tickXFunction(d, 30);
				return self.tickXFunction(d, 12);
			})
			.attr('dy', function(d, i) {
				if(newValue == d)return self.tickYFunction(d, 30);
				return self.tickYFunction(d, 12);
			});
	};

	self.arcTween = function(transition, start, end) {
		transition.attrTween('d', function(d) {
			var interpolateStart = d3.interpolate(d.startAngle, start);
			var interpolateEnd = d3.interpolate(d.endAngle, end);
			return function(t) {
				d.endAngle = interpolateEnd(t);
				d.startAngle = interpolateStart(t);
				return self.arc(d);
			};
		});
	};

	self.refreshPicker(self.progress);
}