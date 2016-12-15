const mongoose = require('mongoose');

var Db = function () {};

Db.prototype.TemperatureHistory = mongoose.model('tempHistory', {
	deviceId : String,
	temp : Number,
	time : Number
});

Db.prototype.Device = mongoose.model('device', {
	deviceId: String,
	currentTemp: String,
	manualTemp: String,
	mode: String,
	autoTemp: String,
	uptime: String
});

module.exports = new Db();