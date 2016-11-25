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
	var mqtt = mqttLib.connect('mqtt://ec2.mrostudios.com');
	mqtt.on('connect', function() {
		mqtt.subscribe('mrostudios/devices/+/+/status');
	});

	mqtt.on('message', function(topic, message) {
		var topicParts = topic.split("/");
		if (topicParts.length != 5) {
			console.log("Topic wrong - " + topic);
			return;
		}

		var deviceId = topicParts[2];
		var attribute = topicParts[3];

		var updatedDevice = {
			"deviceId": deviceId
		};
		updatedDevice[attribute] = message;
		Device.findOneAndUpdate({
				deviceId: deviceId
			}, updatedDevice, {
				upsert: true
			})
			.then(function(doc) {
				console.log('updated entry');
			});
	});
};
