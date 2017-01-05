var listeningPort = 8085;

const config = require('./config.js');
const util = require('util');
const winston = require('./logging.js');
const express = require('express');
const db = require('./dbModel.js');
const mongoose = require('mongoose');

winston.level = 'debug';
winston.cli();

mongoose.connect(config.mongoUrl, {user : config.mongoRestUsername, pass : config.mongoRestPassword});
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
	var query = db.TemperatureHistory
		.find({
			deviceId: req.params.id
		})
		.sort({
			time : 'asc'
		});		

	if (req.query.limit) query = query.limit(parseInt(req.query.limit));
	if (req.query.from) query = query.where({
		time: {
			$gt: parseInt(req.query.from)
		}
	});

	query.exec(function(err, temps) {
		res.write('time,temp\n');
		temps.forEach(function(temp) {			
			res.write(util.format('%d,%d\n', temp.time, temp.temp));
		});
		res.status(200);
		res.end();
	});
});

apiRouter.get('/devices', function(req, res, next) {
	db.Device
		.find({})
		.select({
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
	res.status(500).send('ups');
});

app.listen(listeningPort, function() {
	winston.info('REST service started, listening on port %d', listeningPort);
});
