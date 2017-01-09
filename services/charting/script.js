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
				id: item.name,
				name: item.name,
				data: item.data,
				pointStart: item.data[0][0],
				tooltip: {
					valueDecimals: 1,
					valueSuffix: '°C'
				}});
		});
		//restarts
		var restarts = d[d.length - 1];
		var restartsByDevice = restarts.reduce((last, current) => {
			if(!last[current.deviceId])
				last[current.deviceId] = [];
			last[current.deviceId].push(current.time);
			return last;
		}, {});

		Object.keys(restartsByDevice).forEach(deviceId => {			
			var seriesData = getRestartArray(restartsByDevice[deviceId], deviceId);
			series.push(seriesData);
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
			legend: {
				enabled: true
			},
			yAxis: {
				title: {
					text: 'Temperature (°C)'
				}
			},

			series : series
		});
	});
});

function getRestartArray(restarts, deviceId) {
		var restartData = restarts.reduce((last, it) => {
			if(it > Date.now() - 24 * 60 * 60 * 1000)
				last.push({x : it, title : 'Restart', text: 'Restart'});

			return last;
		}, []);
		return {
            type: 'flags',
            name: 'Restarts - ' + deviceId,
            data: restartData,            
            onSeries: deviceId
		};
}