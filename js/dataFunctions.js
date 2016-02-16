function getDataForESP(dayData) {
	var dataByDay = [];
	_.each(dayData, function(data, i){
		_.each(data.enabledDays, function(value, key){
			if(value){
				dataByDay[key] = data.values;
			}
		});
	});

	var x = _.chain(dataByDay)
		.map(function (d, i){
			return _.map(d, function(item){
				return {
					time: item.time + i * 24,
					temp: item.temp
				};
			});
		})
		.flatten()
		.value();

	return getDataFromBars(x);
}

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
		return data;
	}

	if (data.length == 1) {
		data[0].time = 0;
		data.push({
			time: 24,
			temp: data[0].temp
		});
		return data;
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

	return data;
}

function getStatesByDay(data) {
	dayStates =
		d3.nest()
		.key(function(d) {
			return Math.floor(d.time / 24);
		})
		.entries(data);

	if (dayStates.length == 0)
		dayStates.push({
			values: []
		});

	dayStates.forEach(function(d) {
		//transform minutes to float hours
		d.values.forEach(function(val) {
			val.time -= d.key * 24;
		});

		fixData(d.values);
		d.values = prepareDataForBars(d.values);
	});

	//check if days can be represented by one chart
	var finalData = [];
	var skipI = [];
	for(var i = 0; i < dayStates.length; i++){
		if(_.contains(skipI, i)) continue;
		var currentDay = {
			enabledDays: {},
			values: dayStates[i].values
		};

		finalData.push(currentDay);
		
		currentDay.enabledDays[i] = true;
		for(var j = i + 1; j < dayStates.length; j++){
			if(_.isEqual(dayStates[i].values, dayStates[j].values)){
				currentDay.enabledDays[j] = true;
				skipI.push(j);
			}
		}
	}

	return finalData;
}