deviceUrl = 'http://mrostudios.duckdns.org:23231';

angular.module('thermo', [])	
	.controller('testController', ['$scope', '$timeout', '$http', function($scope, $timeout, $http) {
		$scope.status = {};
		$scope.temperatures = [];
		$scope.activeTab = 'automatic';
		$scope.weatherCity = 'Zagreb';

		$scope.$watch('deleteClick', function(newVal, old) {
			deleteClick = newVal;
		});

		$scope.save = function() {
			var saveData = generateResponse(getDataForESP($scope.dayData));
			$http.post(deviceUrl + '/set_auto_temp', saveData)
				.then(function() {
					$scope.saved = true;
					$timeout(function() {
						$scope.saved = false;
					}, 2000);
					console.log('temp program saved');
				});
		};

		$scope.charts = [];

		$scope.addChart = function() {
			var data = prepareDataForBars([{time:0, temp: 18}]);
			$scope.dayData.push({				
				enabledDays: {},
				values: data
			});
		};

		$scope.getWeather = function(){
			//http://query.yahooapis.com/v1/public/yql?q=select%20item.condition%20from%20weather.forecast%20where%20woeid=851128%20and%20u=%27c%27&format=json
			var yahooApi = 'https://query.yahooapis.com/v1/public/yql?q=select%20item%20from%20weather.forecast%20where%20woeid%20in%20(select%20woeid%20from%20geo.places(1)%20where%20text%3D%22' + $scope.weatherCity + '%22)%20and%20u%3D\'c\'&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys';
			$http.get(yahooApi)
				.then(function(response){
					weather = $scope.weather = response.data.query.results.channel.item;
				});
		};

		$scope.removeChart = function(i) {
			$scope.dayData.splice(i, 1);
		};

		$scope.getStatus = function() {
			$scope.deregisterWatch();
			$http.get(deviceUrl + '/get_status')
				.then(function(response) {
					$scope.status = response.data;
					$scope.registerWatch();
					$scope.activeTab = $scope.status.temperatureMode;
				});
		};

		$scope.refresh = function() {
			$http.get(deviceUrl + '/get_auto_temp')
				.then(function(response) {					
					dayData = $scope.dayData = parseResponse(response.data);

					$scope.getStatus();
				});
		};

		$scope.setOffset = function(offset) {
			$http.post(deviceUrl + '/command', {
				time_offset: offset
			});
		};

		$scope.saveManualTempOnChange =  function(newVal, oldVal){
			if(!newVal || newVal == oldVal) return;

			$timeout.cancel($scope.saveManualTempTimeout);
			$scope.saveManualTempTimeout = $timeout(function(){
				$scope.setManualTemp(newVal);
			}, 2500);
		};

		$scope.setManualTemp = function(temp) {
			$http.post(deviceUrl + '/command', {
				manual_temp: d3.format('.2f')(temp)
			});
			console.log('manual_temp saved: ' + temp);
		};

		$scope.setMode = function(newMode) {
			$scope.activeTab = newMode;
			
			if (newMode == 'automatic') {
				$http.get(deviceUrl + '/set_mode/auto');
			}
			if(newMode == 'manual'){
				$http.get(deviceUrl + '/set_mode/manual');
			}
			$scope.status.temperatureMode = newMode;

		};

		$scope.toggleDay = function(chartData, day) {
			$scope.dayData.forEach(function(d) {
				d.enabledDays[day] = chartData == d;
			});
		};

		$scope.registerWatch = function(){
			$scope.manualTempWatcher = $scope.$watch('status.manual_temp', $scope.saveManualTempOnChange);
		};
		$scope.deregisterWatch = function(){
			if($scope.manualTempWatcher) $scope.manualTempWatcher();
		};

		$scope.$watch('weatherCity', function(newVal, oldVal){
			if(!newVal) return;

			$timeout.cancel(fn);
			var fn = $timeout(function(){
				$scope.getWeather();
			}, 2000);
		});

		$scope.refresh();
		$scope.getWeather();
	}]);