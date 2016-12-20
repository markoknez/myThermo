function parseCSV(csv) {
	var data = [];
	var lines = csv.split('\n');
	for (var i = 1; i < lines.length; i++) {
		var values = lines[i].split(',');
		data.push([parseInt(values[0]), parseFloat(values[1])]);
	}
	return data;
}


function getThermoData(deviceId) {
	return $.get('http://thermo.mrostudios.com/rest/api/temphistory/' + deviceId + '?from=' + (Date.now() - 3600000 * 24)).then(function(csv) {
		return {name : deviceId, data: parseCSV(csv)};
	}, console.error);
}

function getDevices() {
	return $.getJSON('http://thermo.mrostudios.com/rest/api/devices');
}

$(function() {	
	getDevices()
		.then(devices => {
			var promises = [];
			devices.forEach(device => {
				var deviceId = device.deviceId;
				promises.push(getThermoData(deviceId));
			});
			return Promise.all(promises);
		})
		.then(d => {		
		// Create a timer
		var start = +new Date();

		var series = [];
		d.forEach(item => {
			series.push({
				name: item.name,
				data: item.data,
				pointStart: item.data[0][0],
				tooltip: {
					valueDecimals: 1,
					valueSuffix: '°C'
				}});
		});

		// Create the chart
		Highcharts.stockChart('container', {
			chart: {
				events: {
					load: function() {
						this.setTitle(null, {
							text: 'Built chart in ' + (new Date() - start) + 'ms'
						});
					}
				},
				zoomType: 'x'
			},

			rangeSelector: {

				buttons: [{
					type: 'day',
					count: 3,
					text: '3d'
				}, {
					type: 'week',
					count: 1,
					text: '1w'
				}, {
					type: 'month',
					count: 1,
					text: '1m'
				}, {
					type: 'month',
					count: 6,
					text: '6m'
				}, {
					type: 'year',
					count: 1,
					text: '1y'
				}, {
					type: 'all',
					text: 'All'
				}],
				selected: 3
			},

			yAxis: {
				title: {
					text: 'Temperature (°C)'
				}
			},

			title: {
				text: ''
			},

			subtitle: {
				text: ''
			},
			series : series
		});
	});
});