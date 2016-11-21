var client = new(require('node-rest-client').Client)();
var mqttLib = require('mqtt');
var mqtt = mqttLib.connect('mqtt://test.mosquitto.org');
var _ = require('underscore');

var WEATHER_REFRESH_INTERVAL = 5 * 60 * 1000;
var woeids = new Set();


var refreshWeatherForWOEID = function(woeid) {
	var url = "http://query.yahooapis.com/v1/public/yql?q=select%20item.condition%20from%20weather.forecast%20where%20woeid={0}%20and%20u=%27c%27&format=json".replace('{0}', woeid);
	client.get(url, {
		headers: {
			"Content-Type": "application/json"
		}
	}, function(data, response) {		
		if (data && data.query && data.query.count == 1) {
			console.log('Got response for woeid ' + woeid);
			console.log(data.query.results.channel.item.condition);
			var c = data.query.results.channel.item.condition;

			mqtt.publish('mrostudios/weather/' + woeid + '/status', '{1};{2};{3};'.replace('{1}',c.code).replace('{2}',c.temp).replace('{3}',c.text), {retain : true});
		}
		else {
			console.log('Did not get expected data');
			console.log(data.toString());
		}
	}).on('error', function(err) {
		console.log(err);
	});
}

var refreshWeather = function() {
	console.log('Refreshing weather, number of entries ' + woeids.size);
	for(id of woeids) {
		refreshWeatherForWOEID(id);
	}

	setTimeout(refreshWeather, WEATHER_REFRESH_INTERVAL); //every 5 minutes
};

mqtt.on('connect', function() {
	mqtt.subscribe('mrostudios/+/config/weather');	
});

mqtt.on('message', function(topic, messageData) {	
	var message = messageData.toString();
	if(topic.indexOf('config/weather') >= 0) {				
		if(!_.isNaN(parseInt(message))) {
			woeids.add(message);
			console.log('New woeid received: ' + message);
			console.log('Weoids register size: ' + woeids.size);			
		}
	}
});

//wait for retained messages to kick in
setTimeout(refreshWeather, 5000);
