function RestClient() {
	// this.baseURL = 'http://thermo.mrostudios.com/rest';
	this.baseURL = 'http://localhost:8085';
	this.from = Date.now() - 24 * 3600000;
	this.to = Date.now();
};

RestClient.prototype.devices = function() {
	return $.getJSON(this.baseURL + '/api/devices');
};

RestClient.prototype.restarts = function() {
	return $.getJSON(this.baseURL + '/api/restarts');
};

RestClient.prototype.parseCSV = function(csv) {
	var data = [];
	var lines = csv.split('\n');
	for (var i = 1; i < lines.length; i++) {
		if (lines[i].length == 0)
			continue;
		var values = lines[i].split(',');
		data.push([parseInt(values[0]), parseFloat(values[1])]);
	}
	return data;
};

RestClient.prototype.events = function(deviceId, event, isCSV) {
	if (isCSV === true) {
		return $.get(this.baseURL + '/api/events/' + deviceId + '/' + event + '?from=' + this.from + '&to=' + this.to + "&csv=true")
			.then(data => {
				return this.parseCSV(data);
			});
	}
	return $.getJSON(this.baseURL + '/api/events/' + deviceId + '/' + event + '?from=' + this.from + '&to=' + this.to)
};

/* RestClient.prototype.eventsFloat = function(deviceId, event) {
	return $.getJSON(this.baseURL + '/api/events/' + deviceId + '/' + event + '?from=' + this.from + '&to=' + this.to)
		.then(function (data) {
			return data.map(it => {
				return {
					time: it.time,
					value: parseFloat(value)
				};
			});
		});
}*/