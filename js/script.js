stopClick = false;
deleteClick = false;



function chart(element, data) {
	var self = this;
	self.data = data;

	self.addChart(element);
}

chart.prototype.drawBars = function(data) {
	var chart = this.chart;
	this.bars = chart.selectAll('.graph-rect').data(data, function(d) {
		return d.time;
	});

	this.bars.enter()
		.append('rect')
		.attr('class', 'graph-rect')
		.attr('x', function(d) {
			return xRange(d.time) + xRange.rangeBand() / 2 + xRange.rangeBand() / 8;
		})
		.attr('y', HEIGHT - MARGINS.top)
		.attr('width', 0)
		.attr('height', 0)
		.attr('y', function(d) {
			return yRange(d.temp);
		})
		.attr('height', function(d) {
			return HEIGHT - MARGINS.bottom - yRange(d.temp);
		});

	this.bars.transition().delay(function(d, i) {
			return i * 25;
		})
		//.duration(50)
		.attr('width', xRange.rangeBand());
};

chart.prototype.addChart = function(element) {
	var self = this;

	self.chart = d3.select(element)
		.append('svg')
		.attr('height', HEIGHT)
		.attr('width', WIDTH);

	//add axes
	self.chart.append('svg:g')
		.attr('class', 'x axis')
		.attr('transform', 'translate(0,' + (HEIGHT - MARGINS.bottom) + ')')
		.call(xAxis);
	self.chart.append('svg:g')
		.attr('class', 'y axis')
		.attr('transform', 'translate(' + MARGINS.left + ',0)')
		.call(yAxis);


	// self.addGradient();

	self.chart.on('mousemove', function() {
		self.changeBar(this);
	});
	self.chart.on('mousedown', function() {
		self.changeBar(this);
	});

	self.drawBars(self.data);
	self.setCurrentTemp(16);
};

chart.prototype.changeBar = function(element) {
	var self = this;
	if (d3.event.buttons == 0) return;

	d3.event.preventDefault();

	var pos = d3.mouse(element);
	pos[0] -= xRange.rangeBand() / 2;
	var stateIndex = d3.bisect(xRange.range(), pos[0]) - 1;
	if (stateIndex < 0 || stateIndex > self.data.length - 1) return;

	var temp = Math.round(yRange.invert(pos[1]));
	self.bars.filter(function(d, i) {
			return i === stateIndex;
		})
		.attr('y', function(d) {
			return yRange(temp);
		})
		.attr('height', function(d) {
			return HEIGHT - MARGINS.bottom - yRange(temp);
		});

	self.data[stateIndex].temp = Math.round(yRange.invert(pos[1]));
};

chart.prototype.addGradient = function() {
	var chart = this.chart;
	var gradient = chart.append("defs")
		.append("linearGradient")
		.attr("id", "gradient")
		.attr("x1", '0%')
		.attr("y1", '0%')
		.attr("x2", '0%')
		.attr("y2", '100%')
		// .attr("spreadMethod", "pad");
		.attr('gradientUnits', 'userSpaceOnUse');

	gradient.append("stop")
		.attr("offset", "20%")
		.attr("class", 'chart-color-hot');

	gradient.append("stop")
		.attr("offset", "50%")
		.attr("class", "chart-color-medium");

	gradient.append("stop")
		.attr("offset", "80%")
		.attr("class", "chart-color-cold");
};

chart.prototype.addCurrentTime = function() {
	var self = this;
	var chart = self.chart;

	var d = new Date();
	var currentTime = d.getHours() + d.getMinutes() / 60;
	var timeLine = chart.selectAll('.time-line').data([currentTime]);

	timeLine
		.enter()
		.append('line')
		.attr('class', 'time-line')
		.attr('x1', MARGINS.left)
		.attr('x2', MARGINS.left);

	timeLine.transition()
		.duration(2000)
		.attr('visibility', 'visible')
		.attr('x1', xRangeLinear(currentTime))
		.attr('y1', MARGINS.top)
		.attr('x2', xRangeLinear(currentTime))
		.attr('y2', HEIGHT - MARGINS.top);

	//refresh every minute
	self.currentTimeTimeout = setTimeout(function() {
		self.addCurrentTime(chart);
	}, 60 * 1000);
};

chart.prototype.removeCurrentTime = function() {
	var self = this;
	clearTimeout(self.currentTimeTimeout);
	self.chart.selectAll('.time-line')
		.attr('visibility', 'hidden');
};

chart.prototype.setCurrentTemp = function(temp) {
	var chart = this.chart;
	var data = [temp];
	var tempLine = chart.selectAll('.temp-line').data(data);

	tempLine.enter()
		.append('line')
		.attr('class', 'temp-line')
		.attr('y1', HEIGHT - MARGINS.bottom)
		.attr('y2', HEIGHT - MARGINS.bottom);

	tempLine.transition()
		.duration(1000)
		.attr('x1', MARGINS.left)
		.attr('y1', function(d) {
			return yRange(d);
		})
		.attr('x2', MARGINS.left + WIDTH)
		.attr('y2', function(d) {
			return yRange(d);
		});
};


MARGINS = {
	top: 20,
	right: 0,
	bottom: 20,
	left: 20
};
WIDTH = 1042 - MARGINS.left - MARGINS.right;
HEIGHT = 200;
leadingZero = d3.format('02d');

xRangeLinear = d3.scale.linear()
	.domain([0, 24])
	.range([MARGINS.left, WIDTH - MARGINS.left]);
xRange = d3.scale.ordinal()
	.rangeBands([MARGINS.left, WIDTH - MARGINS.left], 0.1)
	.domain(d3.range(0, 24.5, 0.5));
yRange = d3.scale.linear()
	.range([HEIGHT - MARGINS.top, MARGINS.bottom])
	.domain([15, 30])
	.clamp(true);
xAxis = d3.svg.axis()
	.scale(xRange)
	.tickValues(xRange.domain().filter(function(d, i) {
		return !(i % 2);
	}))
	.tickFormat(function(d) {
		return leadingZero(d) + ':00';
	})
	.tickSize(5)
	.outerTickSize(0);
yAxis = d3.svg.axis()
	.scale(yRange)
	.orient('left')
	.innerTickSize(-WIDTH + MARGINS.left * 2)
	.outerTickSize(5)
	.tickSubdivide(true);

//setup gradient scale
$('#gradient').attr('y2',(HEIGHT-MARGINS.top)+'px');