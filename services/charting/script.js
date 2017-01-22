var restClient = new RestClient();

function currentTemp(deviceId) {
	return restClient.events(deviceId, 'currentTemp', true)
		.then(data => {
			return getTemperatureSeries(deviceId, data);
		});
}

function manualTemp(deviceId) {
	return restClient.events(deviceId, 'manualTemp')
		.then(data => {
			return getAreaSeries(deviceId + ' - set temp', convertWeirdTemperatures(data));
		});
}

function restarts(deviceId) {
	return restClient.events(deviceId, 'restart')
		.then(data => {
			return getFlagSeries(deviceId, 'R', deviceId + ' - restarts', data);
		});
}

function heater(deviceId) {
	return restClient.events(deviceId, 'heater')
		.then(data => {
			return getFlagSeries(deviceId, 'H', deviceId + ' - heater', data.filter(it => it.value == 'true'));
		});
}

$(function() {
	restClient
		.devices()
		.then(it => {
			return [{
				deviceId: 'termo-2'
			}];
		})
		.then(devices => {
			var promises = [];
			devices.forEach(device => {
				promises.push(currentTemp(device.deviceId));
				promises.push(manualTemp(device.deviceId));
				promises.push(restarts(device.deviceId));
				promises.push(heater(device.deviceId));
			});
			return Promise.all(promises);
		})
		.then(series => {

			// Create the chart
			Highcharts.stockChart('container', {
				chart: {
					zoomType: 'x'
				},
				legend: {
					enabled: true
				},
				xAxis: {
					ordinal: false
				},
				yAxis: {
					title: {
						text: 'Temperature (°C)'
					},
					min: 18
				},

				series: series
			});
		});
});

function getTemperatureSeries(name, data) {
	return {
		id: name,
		name: name,
		data: data,
		tooltip: {
			valueDecimals: 1,
			valueSuffix: '°C'
		}
	};
}

function getFlagSeries(onSeries, flagTitle, name, eventData) {
	var data = eventData.reduce((last, it) => {
		last.push({
			x: it.time,
			title: flagTitle,
			text: name
		});

		return last;
	}, []);
	return {
		type: 'flags',
		name: name,
		data: data,
		onSeries: onSeries
	};
}

function getAreaSeries(name, eventData) {
	var result = {
		type: 'area',
		name: name,
		step: 'left',
		fillOpacity: 0,
		data: eventData.map(it => {
			return {
				x: it.time,
				y: it.value
			};
		})
	};

	result.data.push({
		x: Date.now(),
		y: result.data[result.data.length - 1].y
	});

	return result;
}

function convertWeirdTemperatures(dataArray) {
	return dataArray.reduce((last, it) => {
		last.push({
			time: it.time,
			value: parseFloat(it.value) / 100
		});
		return last;
	}, []);
}