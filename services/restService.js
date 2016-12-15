var listeningPort = 8085;

const winston = require('./logging.js');
const express = require('express');
const db = require('./dbModel.js');
const mongoose = require('mongoose');

winston.level = 'debug';
winston.cli();

mongoose.connect('mongodb://localhost/thermo');
var dbConnection = mongoose.connection;
dbConnection.on('error', function(err) {
	winston.error("Error connecting to mongodb %j", err);
});
dbConnection.once('open', function() {
	winston.info("Connected to mongodb");
});

const app = express();
const apiRouter = express.Router();

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