var restClient = new RestClient();
var chart = {};

$(function() {
	$('#loading').hide();
	var now = new Date();
	now = new Date(now.getFullYear(), now.getMonth(), now.getDate());

	restClient.setFrom(now.getTime());

	// Create the chart
	chart = Highcharts.stockChart('container', {
		chart: {
			zoomType: 'x'
		},
		legend: {
			enabled: true,
			layout: 'vertical',
			align: 'left',
			verticalAlign: 'middle'
		},
		// navigator: {
		// 	enabled: false
		// },
		xAxis: {
			ordinal: false
		},
		yAxis: {
			title: {
				text: 'Temperature (°C)'
			},
			min: 18
		}
	});
	loadAllCharts(chart);

	$('#date').val(now.getFullYear() + '-' + (now.getMonth() + 1) + '-' + now.getDate());

	$('#date').change(() => {
		var date = new Date($('#date').val());
		if (date == 'Invalid Date')
			return;
		restClient.setFrom(date.getTime());

		loadAllCharts(chart);
	});

	restClient.devices().then(devices => {
		devices.forEach(device => {
			$('#deviceSelector').append('<option>' + device.deviceId + '</option>');
		});
		$('#deviceSelector').change(() => {
			loadAllCharts(chart);
		});
	});


});

function loadAllCharts(chart) {
	var selectedDevice = $('#deviceSelector').val();
	chart.xAxis[0].setExtremes(restClient.from, restClient.to);

	while (chart.series.length > 0) {
		chart.series[0].remove();
	}

	restClient
		.devices()
		.then(devices => {
			var promises = [];
			devices.filter(it => (selectedDevice == 'all' || it.deviceId == selectedDevice))
				.forEach(device => {
					promises.push(currentTemp(chart, device.deviceId).then(() => {
						promises.push(restarts(chart, device.deviceId));
						promises.push(heater(chart, device.deviceId));
						promises.push(errors(chart, device.deviceId));
					}));
					promises.push(manualTemp(chart, device.deviceId));
				});
			return Promise.all(promises);
		});
}

function getTemperatureSeries(name, data) {
	return {
		type: 'line',
		id: name,
		name: name,
		data: data,
		tooltip: {
			valueDecimals: 1,
			valueSuffix: '°C'
		},
		showInNavigator: true
	};
	c
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
		}),
		showInNavigator: false
	};

	if (result.data.length > 0) {
		result.data.push({
			x: restClient.to,
			y: result.data[result.data.length - 1].y
		});
		result.data.unshift({
			x: restClient.from,
			y: result.data[0].y
		});
	}

	return result;
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


function convertWeirdTemperatures(dataArray) {
	return dataArray.reduce((last, it) => {
		last.push({
			time: it.time,
			value: parseFloat(it.value) / 100
		});
		return last;
	}, []);
}

function addLoadingMessage(msg) {
	var div = document.createElement('div');
	div.innerText = msg;
	$('#loading').append(div);
}

function currentTemp(chart, deviceId) {
	return restClient.events(deviceId, 'currentTemp', true)
		.then(data => {
			addLoadingMessage('loaded currentTemp data for ' + deviceId);
			chart.addSeries(getTemperatureSeries(deviceId, data));
		});
}

function manualTemp(chart, deviceId) {
	return restClient.events(deviceId, 'manualTemp')
		.then(data => {
			addLoadingMessage('loaded manualTemp data for ' + deviceId);
			chart.addSeries(getAreaSeries(deviceId + ' - set temp', convertWeirdTemperatures(data)));
		});
}

function restarts(chart, deviceId) {
	return restClient.events(deviceId, 'restart')
		.then(data => {
			addLoadingMessage('loaded restart events for ' + deviceId);
			chart.addSeries(getFlagSeries(deviceId, 'R', deviceId + ' - restarts', data));
		});
}

function heater(chart, deviceId) {
	return restClient.events(deviceId, 'heater')
		.then(data => {
			addLoadingMessage('loaded heater events for ' + deviceId);
			chart.addSeries(getFlagSeries(deviceId, 'H', deviceId + ' - heater', data.filter(it => it.value == 'true')));
		});
}

function errors(chart, deviceId) {
	return restClient.events(deviceId, 'error')
		.then(data => {
			addLoadingMessage('loaded errors for ' + deviceId);
			chart.addSeries(getFlagSeries(deviceId, 'E', deviceId + ' - error', data));
		});
}