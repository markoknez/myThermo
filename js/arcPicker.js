Math.Tau = 2 * Math.PI;

function arcPicker(element, isPicker, $scope) {
	var self = this;

	self.min = 15;
	self.max = 30;

	var tweenDuration = 500;
	var width = 170;
	var radius = 50;
	var arcWidth = 30;
	var height = width;
	self.end = 0;

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
		.datum({
			startAngle: 0,
			endAngle: Math.Tau
		})
		.style('fill', '#ddd')
		.attr('d', self.arc);

	self.foreground = self.svg.append('path')
		.datum({
			startAngle: 0,
			endAngle: Math.Tau * self.end
		})
		.style('fill', 'orange')
		.attr('d', self.arc);

	self.pickerPart = self.svg.append('path')
		.datum({
			startAngle: 0,
			endAngle: 0
		})
		.style('fill', 'black')
		.attr('visibility', 'hidden')
		.attr('d', self.arc);

	self.textUpper = self.container.append('text')
		.attr('class', 'arc-inner-text')
		.attr('text-anchor', 'middle')
		.attr('alignment-baseline', 'middle')
		.attr('dx', '50%')
		.attr('dy', '50%');

	if (isPicker == true) {
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
			var newAngle = convertValueToAngle(Math.round(convertAngleToValue(self.getEndAngleFromMouse())));
			self.pickerPart.datum({startAngle: Math.Tau * newAngle - 0.04, endAngle: Math.Tau * newAngle + 0.04}).attr('d', self.arc);

			if (!self.mousedown) return;
			self.refreshPicker(newAngle);
		});
	}

	function convertAngleToValue(angle) {
		return self.min + (self.max - self.min) * angle;
	};

	function convertValueToAngle(value) {
		return (value - self.min) / (self.max - self.min);
	};

	self.setValue = function(value) {
		var newAngle = convertValueToAngle(Math.round(value));
		self.refreshPicker(newAngle);
	};

	self.getEndAngleFromMouse = function() {
		var pos = d3.mouse(self.foreground.node());
		var angle = (360 / Math.Tau * Math.atan2(pos[1], pos[0])) + 360 + 90;
		return (angle % 360) / 360;
	};

	self.refreshPicker = function(newAngle) {
		var newValue = Math.round(convertAngleToValue(newAngle));
		var newEndValue = convertValueToAngle(newValue);
		if ($scope && newEndValue != self.end) {
			console.log(newValue);
			$scope.$emit('valueChanged', newValue);
		}
		self.end = newEndValue;

		self.foreground
			.transition()
			.duration(tweenDuration)
			.call(self.arcTween, 0, Math.Tau * self.end);

		self.textUpper.html(function() {
			return newValue + 'c';
		});
	};

	self.arcTween = function(transition, start, end){
		transition.attrTween('d', function(d){
			var interpolateStart = d3.interpolate(d.startAngle, start);
			var interpolateEnd = d3.interpolate(d.endAngle, end);
			return function(t){
				d.endAngle = interpolateEnd(t);
				d.startAngle = interpolateStart(t);
				return self.arc(d);
			};
		});
	};

	self.refreshPicker(self.end);
}