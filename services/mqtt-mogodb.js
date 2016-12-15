var mongoose = require('mongoose');
var mqttLib = require('mqtt');
var winston = require('winston');

winston.level = 'debug';
winston.cli();

mongoose.connect('mongodb://localhost/thermo');
var db = mongoose.connection;
db.on('error', function (err) { winston.error("Error connecting to mongodb. %j", err); });
db.once('open', function() {
	winston.info("Connected to mongodb");
	setupMqtt();
});

var TemperatureHistory = mongoose.model('tempHistory', {
	deviceId : String,
	temp : Number,
	time : Number
});

var Device = mongoose.model('device', {
	deviceId: String,
	currentTemp: String,
	manualTemp: String,
	mode: String,
	autoTemp: String,
	uptime: String,
	tempHistory: []
});

var writeCurrentTempToDB = function(deviceId, currentTemp) {	
	winston.debug("Writing temp to history: %s, %s", deviceId, currentTemp);
	TemperatureHistory.create({
		deviceId: deviceId,
		temp : currentTemp,
		time : Date.now()
	}).then(null, function(err) { winston.error(err); });
};

var setupMqtt = function() {
	var mqtt = mqttLib.connect('mqtt://ec2.mrostudios.com');
	mqtt.on('connect', function() {
		winston.info("Connected to mqtt");
		mqtt.subscribe('mrostudios/devices/+/+/status');
	});

	mqtt.on('message', function(topic, messageData) {
		var message = messageData.toString();
		winston.debug("Got mqtt message %s, %s", topic, message);
		var topicParts = topic.split("/");
		if (topicParts.length != 5) {
			winston.error("Topic wrong - " + topic + " , message - " + message);
			return;
		}
		if(topicParts[3] == 'currentTemp') { 
			var parsedTemperature = parseFloat(message);
			if(parsedTemperature != NaN)
				writeCurrentTempToDB(topicParts[2], parsedTemperature);
		}

		var deviceId = topicParts[2];
		var attribute = topicParts[3];

		var updatedDevice = {
			"deviceId": deviceId
		};
		updatedDevice[attribute] = message;
		winston.debug("Updating device, %s, %j", deviceId, updatedDevice);
		Device.findOneAndUpdate({
				deviceId: deviceId
			}, updatedDevice, {
				upsert: true
			})
			.then(null, function(err) { winston.error(err); });
	});
};