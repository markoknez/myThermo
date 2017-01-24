var config = require('./config.js');
var mongoose = require('mongoose');
var db = require('./dbModel.js');
var log = require('./logging.js');

mongoose.connect(config.mongoUrl, {
	user: config.mongoMqttUsername,
	pass: config.mongoMqttPassword
});

var dbConnection = mongoose.connection;
dbConnection.on('error', function(err) {
	log.error("Error connecting to mongodb %j", err);
});
dbConnection.once('open', function() {
	log.info("Connected to mongodb");
	doWork();
});

function doWork() {
	var i = 0;
	db.Events.find({
			deviceId: 'termo-1',
			attribute: {
				$in: ['currentTemp']
			},
			time: {
				$lt: 1485193169870
			}
		}).cursor()
		.on('data', it => {
			if (i % 1000 == 0)
				log.info('parsed %j', i);
			var number = parseFloat(it.value);			
			number += 3.75;			
			it.value = number;			
			it.save();
			i++;
		})
		.on('end', () => {
			log.info('finished');
		})
		.on('error', (err) => {
			log.error(err);
		});
}