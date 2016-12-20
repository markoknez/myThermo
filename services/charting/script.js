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
		return parseCSV(csv);
	}, console.error);
}

$(function() {
	var d = {};
	getThermoData('termo-1')
		.then(function (data) {
			d.t1 = data;
			return getThermoData('termo-2');
		})
		.then(function (data) {
			d.t2 = data;
			return d;			
		})
		.then(function (d) {		

		// Create a timer
		var start = +new Date();

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

			series: [{
				name: 'termo-1',
				data: d.t1,
				pointStart: d.t1[0][0],
				tooltip: {
					valueDecimals: 1,
					valueSuffix: '°C'
				}
			}, {
				name: 'termo-2',
				data: d.t2,
				pointStart: d.t2[0][0],
				tooltip: {
					valueDecimals: 1,
					valueSuffix: '°C'
				}
			}]

		});
	});
});