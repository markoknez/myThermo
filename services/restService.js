var listeningPort = 8085;

const config = require('./config.js');
const util = require('util');
const winston = require('./logging.js');
const express = require('express');
const db = require('./dbModel.js');
const mongoose = require('mongoose');

winston.level = 'debug';
winston.cli();

mongoose.connect(config.mongoUrl, {
	user: config.mongoRestUsername,
	pass: config.mongoRestPassword
});
var dbConnection = mongoose.connection;
dbConnection.on('error', function(err) {
	winston.error("Error connecting to mongodb %j", err);
});
dbConnection.once('open', function() {
	winston.info("Connected to mongodb");
});

const app = express();
const apiRouter = express.Router();

apiRouter.use(function(req, res, next) {
	res.header("Access-Control-Allow-Origin", "*");
	res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
	next();
});

apiRouter.get('/tempHistory/:id', function(req, res, next) {
	var query = db.Events
		.find({
			deviceId: req.params.id,
			attribute: 'currentTemp'
		})
		.sort({
			time: 'asc'
		});

	if (req.query.limit) query = query.limit(parseInt(req.query.limit));
	if (req.query.from) query = query.where({
		time: {
			$gt: parseInt(req.query.from)
		}
	});

	if (!req.query.limit && !req.query.from)
		query = query.limit(100);

	query.exec(function(err, temps) {
		res.write('time,temp\n');
		temps.forEach(function(temp) {
			res.write(util.format('%d,%d\n', temp.time, temp.value));
		});
		res.status(200);
		res.end();
	});
});

apiRouter.get('/events/:deviceId/:attribute', function(req, res, next) {
	var deviceId = req.params.deviceId;
	var attribute = req.params.attribute;
	var from = parseInt(req.query.from);
	var to = parseInt(req.query.to);
	var isCSV = req.query.csv == 'true';

	if (!deviceId) {
		return next(new Error('deviceId required'));
	}
	if (!attribute) {
		return next(new Error('attribute required'));
	}
	if (from == NaN) {
		return next(new Error('from required'));
	}
	if (isNaN(to)) {
		to = from + 24 * 60 * 60 * 1000;
	}

	var query = db.Events
		.find({
			attribute: attribute,
			deviceId: deviceId,
			time: {
				$gte: from,
				$lt: to
			}
		})
		.select({
			_id: 0,
			time: 1,
			value: 1
		});

	if (isCSV) {
		var stream = query.stream();
		res.write('time,temp\n');
		stream.on('data', data => {
			res.write(util.format('%d,%d\n', data.time, data.value));
		});
		stream.on('error', (err) => {
			next(err);
		});
		stream.on('close', () => {
			res.end();
		});
		return;
	}

	return query.exec(function(err, data) {
		if (err) return next(err);
		res.json(data);
	});
});

apiRouter.get('/restarts', function(req, res, next) {
	db.Events
		.find({
			attribute: 'restart'
		})
		.select({
			_id: 0,
			deviceId: 1,
			time: 1
		})
		.exec(function(err, data) {
			if (err) return next(err);
			data.map(it => {
				return {
					deviceId: it.deviceId,
					time: it.time - 50000
				};
			});
			res.json(data);
		});
});

apiRouter.get('/devices', function(req, res, next) {
	db.Device
		.find({})
		.select({
			_id: 0,
			deviceId: 1
		})
		.exec(function(err, devices) {
			if (err) return next(err);
			res.json(devices);
		});
});

apiRouter.get('/devices/:id', function(req, res, next) {
	var deviceId = req.params.id;
	db.Device
		.findOne({
			deviceId: deviceId
		})
		.exec(function(err, device) {
			if (err) return next(err);

			if (!device) {
				res.status(404).send('Not found');
				return;
			}
			res.json(device);
		});
});

app.use('/api', apiRouter);

app.use(function(err, req, res, next) {
	winston.error(err);
	res.status(500).send(err.message);
});

app.listen(listeningPort, function() {
	winston.info('REST service started, listening on port %d', listeningPort);
});