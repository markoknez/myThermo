var defaultData = [{time: 0, temp: 18}, {time: 50, temp: 20}];

function getDataForESP(dayData) {
	var dataByDay = [];
	_.each(dayData, function(data, i) {
		_.each(data.enabledDays, function(value, key) {
			if (value) {
				dataByDay[key] = data.values;
			}
		});
	});

	var x = _.chain(dataByDay)
		.map(function(d, i) {
			return _.map(d, function(item) {
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

/**
 * Returns 48 points for filling out whole bar chart
*/
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

function generateResponse(data) {
	var response = [];
	response.push(data.length >> 8);
	response.push(data.length & 0xff);
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

/**
 * Converts base64 message from esp8266/get_auto_temp
 * to set of points {time[hour.float], temp[celsius.float]}
 */
function convertBase64ToPoints(data) {
	var counter = 0;

	var x = base64js.toByteArray(data);
	var totalStateCount = (x[counter++] << 8) | x[counter++];

	var points = [];
	for (var stateIndex = 0; stateIndex < totalStateCount; stateIndex++) {
		var state = {
			time: 0,
			temp: 0
		};
		state.time = (x[counter++] << 8) | x[counter++];
		state.temp = (x[counter++] << 8) | x[counter++];
		state.time = state.time / 60;
		state.temp = state.temp / 100;
		points.push(state);
	}

	return points;

}

function getStatesByDay(data) {
	//if there is no data, return default 
	if (!data || data.length == 0) data = defaultData;

	//group states by day
	var dayStates =
		d3.nest()
		.key(function(d) {
			return Math.floor(d.time / 24);
		})
		.entries(data);

	//check if days are missing
	var lastTemp = _.last(_.last(dayStates).values).temp;
	for(var i = 0; i < 7; i++){
		if(!dayStates[i] || dayStates[i].key != i){
			dayStates.splice(i, 0, {
				key: i + '',
				values: [{time: i * 24, temp: lastTemp}]
			});
		} else {
			lastTemp = _.last(dayStates[i].values).temp;			
		}
	}

	dayStates.forEach(function(d, i) {
		//offset each day to have relative timespace - day starts from time = 0
		d.values.forEach(function(val) {
			val.time -= d.key * 24;
		});

		//check if day has starting temp, if it does not have, take one from day before
		if (d.values[0].time != 0) {
			var lastDayLastTemp = _.last(_.last(dayStates).values).temp;
			if (i > 0) {
				lastDayLastTemp = _.last(dayStates[i - 1].values).temp;
			}

			d.values.splice(0, 0, {
				time: 0,
				temp: lastDayLastTemp
			});
		}

		d.values = prepareDataForBars(d.values);
	});

	var finalData = [];
	_.each(dayStates, function(d){
		var indexOf = indexOfItem(finalData, d.values);
		if(indexOf == -1){
			var newData = {
				enabledDays: {},
				values: d.values
			};
			newData.enabledDays[d.key] = true;
			finalData.push(newData);
			return;
		}

		finalData[indexOf].enabledDays[d.key] = true;
	});

	return finalData;
}

function indexOfItem(list, value){	
	for(var i = 0; i < list.length; i++){
		if(_.isEqual(list[i].values, value))
			return i;
	}	
	return -1;
}

/**
 * Parses base64 response from esp8266/get_auto_temp and returns
 * array of charts [{values:[{time,temp}], enabledDays:{0:true, 1:false...}}]
 */
function parseResponse(base64) {
	var points = convertBase64ToPoints(base64);

	var dayData = getStatesByDay(points);

	return dayData;
}