var mqttLib = require('mqtt');
var wt = require('node-notifier').WindowsToaster;
wt = wt({});

var mqtt = mqttLib.connect('mqtt://test.mosquitto.org');
mqtt.on('connect', function() {
	mqtt.subscribe('mrostudios/devices/+/uptime/status');
});
mqtt.on('message', function(topic, message) {
	if(parseInt(message) < 60)
	wt.notify({title: topic.toString(), message: message.toString(), sound: true});
});