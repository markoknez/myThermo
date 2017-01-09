function RestClient() {
	this.baseURL = 'http://localhost:8085';
};

RestClient.prototype.tempData = function(deviceId) {
	var self = this;
	return $.get(this.baseURL + '/api/temphistory/' + deviceId + '?from=' + (Date.now() - 3600000 * 24)).then(function(csv) {
		return {
			name: deviceId,
			data: self.parseCSV(csv)
		};
	});
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
		var values = lines[i].split(',');
		data.push([parseInt(values[0]), parseFloat(values[1])]);
	}
	return data;
};