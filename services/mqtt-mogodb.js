var config = require('./config.js');
var mongoose = require('mongoose');
var db = require('./dbModel.js');
var mqttLib = require('mqtt');
var winston = require('./logging.js');

mongoose.connect('mongodb://localhost/thermo');
var dbConnection = mongoose.connection;
dbConnection.on('error', function (err) { winston.error("Error connecting to mongodb %j", err); });
dbConnection.once('open', function() {
	winston.info("Connected to mongodb");
	setupMqtt();
});



var writeCurrentTempToDB = function(deviceId, currentTemp) {	
	winston.debug("Writing temp to history: %s, %s", deviceId, currentTemp);
	db.TemperatureHistory.create({
		deviceId: deviceId,
		temp : currentTemp,
		time : Date.now()
	}).then(null, function(err) { winston.error(err); });
};

var setupMqtt = function() {
	var mqtt = mqttLib.connect('mqtt://ec2.mrostudios.com', {username : config.mqttMongodbUsername, password : config.mqttMongodbPassword});
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
		if(topicParts[3] != 'uptime') {
			db.Events.create({deviceId : topicParts[2], time : Date.now(), attribute : topicParts[3], value : message}).then(null, function(err) { winston.error(err); });
		}
		if(topicParts[3] == 'uptime' && message == '50') {
			db.Events.create({deviceId : topicParts[2], time : Date.now(), attribute : 'restart', value : 'true'}).then(null, function(err) { winston.error(err); });
		}

		var deviceId = topicParts[2];
		var attribute = topicParts[3];

		var updatedDevice = {
			"deviceId": deviceId
		};
		updatedDevice[attribute] = message;
		winston.debug("Updating device, %s, %j", deviceId, updatedDevice);
		db.Device.findOneAndUpdate({
				deviceId: deviceId
			}, updatedDevice, {
				upsert: true
			})
			.then(null, function(err) { winston.error(err); });
	});
};
