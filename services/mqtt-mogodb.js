var mongoose = require('mongoose');
var mqttLib = require('mqtt');

mongoose.connect('mongodb://localhost/thermo');
var db = mongoose.connection;
db.on('error', console.error.bind(console, 'connection error:'));
db.once('open', function() {
	setupMqtt();
});


var Device = mongoose.model('device', {
	deviceId: String,
	currentTemp: String,
	manualTemp: String,
	mode: String,
	autoTemp: String,
	uptime: String
});


var setupMqtt = function() {
	var mqtt = mqttLib.connect('mqtt://test.mosquitto.org');
	mqtt.on('connect', function() {
		mqtt.subscribe('mrostudios/devices/+/+/status');
	});

	mqtt.on('message', function(topic, message) {
		var topicParts = topic.split("/");
		if (topicParts.len != 4)
			console.log("Topic wrong - " + topic);
		var deviceId = topic[2];
		var attribute = topic[3];

		var updatedDevice = {
			"deviceId": deviceId
		};
		updatedDevice[attribute] = message;
		Device.findOneAndUpdate(updatedDevice)
			.then(function (doc) {

			});
	});
};