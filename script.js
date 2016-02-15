stopClick = false;
deleteClick = false;


function prepareDataForBars(data) {
	var states = [];

	var dataIndex = 0;
	for (var i = 0; i < 48; i++) {
		if (dataIndex + 1 < data.length && data[dataIndex + 1].time <= i / 2) dataIndex++;

		states.push({
			time: i / 2,
			temp: data[dataIndex].temp
		});
	}

	return states;
}


function getDataFromBars(states) {
	var data = [];
	var lastTemp;
	for (var i = 0; i < states.length; i++) {
		if (states[i].temp != lastTemp) {
			data.push({
				time: states[i].time,
				temp: states[i].temp
			});
			lastTemp = states[i].temp;
		}
	}
	return data;
}

function parseResponse(data) {
	var counter = 0;

	var x = base64js.toByteArray(data);
	var totalStateCount = x[counter++];

	var allStates = [];
	for (var stateIndex = 0; stateIndex < totalStateCount; stateIndex++) {
		var state = {
			time: 0,
			temp: 0
		};
		state.time = (x[counter++] << 8) | x[counter++];
		state.temp = (x[counter++] << 8) | x[counter++];
		state.time = state.time / 60;
		state.temp = state.temp / 100;
		allStates.push(state);
	}

	return allStates;
}

function generateResponse(data) {
	var response = [];
	response.push(data.length);
	for (var i = 0; i < data.length; i++) {
		var time = data[i].time * 60;
		response.push(time >> 8);
		response.push(time & 0xff);
		var temp = data[i].temp * 100;
		response.push(temp >> 8);
		response.push(temp & 0xff);
	}

	return base64js.fromByteArray(response);
}

function fixData(data) {
	if (data.length == 0) {
		data.push({
			time: 0,
			temp: 18
		}, {
			time: 24,
			temp: 18
		});
		return;
	}

	if (data.length == 1) {
		data[0].time = 0;
		data.push({
			time: 24,
			temp: data[0].temp
		});
		return;
	}

	if (data[0].time != 0) {
		data[0].time = 0;
	}
	if (data[data.length - 1].time != 24) {
		data.push({
			time: 24,
			temp: data[data.length - 1].temp
		})
	}
}

function getStatesByDay(data) {
	var dayStates =
		d3.nest()
		.key(function(d) {
			return Math.floor(d.time / 1440);
		})
		.entries(data);

	if (dayStates.length == 0)
		dayStates.push({
			values: []
		});

	dayStates.forEach(function(d) {
		//transform minutes to float hours
		d.values.forEach(function(val) {
			val.time -= d.key * 1440;
		});

		fixData(d.values);

	});
	return dayStates;
}

function chart(data) {
	var self = this;
	self.data = data;

	self.addChart();
}

chart.prototype.refreshBars = function(data) {
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
		.attr('width', xRange.rangeBand())
		.attr('y', HEIGHT - MARGINS.top)
		.attr('height', 0);

	this.bars.transition()
		.attr('y', function(d) {
			return yRange(d.temp);
		})
		.attr('height', function(d) {
			return HEIGHT - MARGINS.bottom - yRange(d.temp);
		});
};

chart.prototype.addChart = function() {
	var self = this;
	
	self.chart = d3.select('#chartPlaceholder')
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

	self.addCurrentTime();
	self.addGradient();

	self.chart.on('mousemove', function() {
		if (d3.event.buttons == 0) return;

		d3.event.preventDefault();

		var pos = d3.mouse(this);
		var stateIndex = d3.bisect(xRange.range(), pos[0]) - 1;
		if (stateIndex < 0 || stateIndex > self.states.length - 1) return;

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

		self.states[stateIndex].temp = Math.round(yRange.invert(pos[1]));
	});

	self.states = prepareDataForBars(self.data);
	self.setCurrentTemp( 16);
	self.refreshBars(self.states);
}

chart.prototype.addGradient = function() {
	var chart = this.chart;
	var gradient = chart.append("defs")
		.append("linearGradient")
		.attr("id", "gradient")
		.attr("x1", '0%')
		.attr("y1", '0%')
		.attr("x2", '0%')
		.attr("y2", '100%')
		// .attr("spreadMethod", "pad")
		.attr('gradientUnits', 'userSpaceOnUse');

	gradient.append("stop")
		.attr("offset", "20%")
		.attr("stop-color", "#E21906")
		.attr("stop-opacity", 1);

	gradient.append("stop")
		.attr("offset", "50%")
		.attr("stop-color", "#20A50B")
		.attr("stop-opacity", 1);

	gradient.append("stop")
		.attr("offset", "80%")
		.attr("stop-color", "#00C7EA")
		.attr("stop-opacity", 1);
}

chart.prototype.addCurrentTime = function() {
	var chart = this.chart;

	var d = new Date();
	var currentTime = d.getHours() + d.getMinutes() / 60;
	var timeLine = chart.selectAll('.time-line').data([currentTime]);

	timeLine
		.enter()
		.append('line')
		.attr('class', 'time-line');

	timeLine.transition()
		.attr('x1', xRangeLinear(currentTime))
		.attr('y1', MARGINS.top)
		.attr('x2', xRangeLinear(currentTime))
		.attr('y2', HEIGHT - MARGINS.top);

	//refresh every minute
	setTimeout(function() {
		addCurrentTime(chart);
	}, 60 * 1000);
}

chart.prototype.setCurrentTemp = function(temp) {
	var chart = this.chart;
	var data = [temp];
	var tempLine = chart.selectAll('.temp-line').data(data);

	tempLine.enter()
		.append('line')
		.attr('class', 'temp-line');

	tempLine.transition()
		.attr('x1', MARGINS.left)
		.attr('y1', function(d) {
			return yRange(d);
		})
		.attr('x2', MARGINS.left + WIDTH)
		.attr('y2', function(d) {
			return yRange(d);
		})
}


WIDTH = $('#chartPlaceholder').width();
HEIGHT = 200;
MARGINS = {
	top: 20,
	right: 20,
	bottom: 20,
	left: 50
};
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
