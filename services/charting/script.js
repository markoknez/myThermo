$(function() {	
	var restClient = new RestClient();
	restClient
		.devices()
		.then(devices => {
			var promises = [];
			devices.forEach(device => {
				var deviceId = device.deviceId;
				promises.push(restClient.tempData(deviceId));
			});
			promises.push(restClient.restarts());
			return Promise.all(promises);
		})
		.then(d => {		
		// Create a timer
		var start = +new Date();

		var series = [];
		//foreach all devices
		var deviceData = d.slice(0, d.length - 1);
		deviceData.forEach(item => {
			series.push({
				name: item.name,
				data: item.data,
				pointStart: item.data[0][0],
				tooltip: {
					valueDecimals: 1,
					valueSuffix: '°C'
				}});
		});
		//restarts
		var restarts = [];
		d[d.length - 1].forEach(function (item) {
			restarts.push([item.time, 1]);
		});
		series.push({
			name: 'restarts',
			data: restarts.data,
			pointStart: restarts.data[0][0],
			marker: {
				symbol: 'circle'
			}
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