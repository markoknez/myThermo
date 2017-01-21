$(function() {
	var restClient = new RestClient();
	restClient
		.devices()
		.then(it => {
			return [{
				deviceId: 'termo-2'
			}];
		})
		.then(devices => {
			var promises = [];
			promises.push(devices);
			devices.forEach(device => {
				var deviceId = device.deviceId;
				promises.push(restClient.tempData(deviceId));
				promises.push(restClient.events(deviceId, 'manualTemp'));
				promises.push(restClient.events(deviceId, 'restart'));
				promises.push(restClient.events(deviceId, 'heater'));
			});
			return Promise.all(promises);
		})
		.then(d => {
			var deviceData = [];
			var devices = d[0];
			var magicNumber = 4;
			for (var i = 1; i < d.length; i += magicNumber) {
				var deviceIdx = Math.floor(i / magicNumber);

				var currentDeviceData = {};
				currentDeviceData.name = devices[deviceIdx].deviceId;
				currentDeviceData.tempData = d[i].data;
				currentDeviceData.manualTemp = d[i + 1];
				currentDeviceData.restarts = d[i + 2];
				currentDeviceData.heater = d[i + 3].reduce((last, it) => {
					if (it.value == 'true')
						last.push(it);
					return last;
				}, []);

				deviceData.push(currentDeviceData);
			}


			var series = [];

			deviceData.forEach(data => {
				//temperature data
				series.push(getTemperatureSeries(data.name, data.tempData));
				series.push(getFlagSeries(data.name, 'R', data.name + ' - restarts', data.restarts));
				series.push(getAreaSeries(data.name + ' - set temp', data.manualTemp.map(it => {
					return {
						time: it.time,
						value: parseInt(it.value) / 100
					};
				})));
				series.push(getFlagSeries(data.name, 'H', data.name + ' - heater on', data.heater));
			});


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
	return {
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
}