const mongoose = require('mongoose');

var Db = function () {};

Db.prototype.Events = mongoose.model('events', {
	deviceId : String,
	time : Number,
	attribute : String,
	value : String,
	parsed : String
});

Db.prototype.Device = mongoose.model('device', {
	deviceId: String,
	currentTemp: String,
	manualTemp: String,
	mode: String,
	autoTemp: String,
	uptime: String,
	heater: String
});

module.exports = new Db();
