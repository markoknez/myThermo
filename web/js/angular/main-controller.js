deviceUrl = 'http://mrostudios.duckdns.org:23231';

angular.module('thermo', [])	
	.factory('debounce', function($timeout) {
	    return function(callback, interval) {
	        var timeout = null;
	        return function() {
	            $timeout.cancel(timeout);
	            var args = arguments;
	            timeout = $timeout(function () { 
	                callback.apply(this, args); 
	            }, interval);
	        };
	    }; 
	})
	.controller('testController', ['$scope', '$timeout', '$http', function($scope, $timeout, $http) {
		$scope.status = {};
		$scope.temperatures = [];
		$scope.activeTab = '';
		$scope.weatherWOEID;
		$scope.weatherCity;

		function deviceUnreachableHandler(){
			alert('cannot reach device');
			location.reload();
		}

		$scope.save = function() {
			saveData = generateResponse(getDataForESP($scope.dayData));
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

		$scope.getCity = function(){
			$http.get("https://query.yahooapis.com/v1/public/yql?q=select%20name%20from%20geo.places%20where%20woeid=" + $scope.weatherWOEID + "&format=json")
				.then(function(response){
					$scope.weatherCity = response.data.query.results.place.name;
				});
		};

		$scope.getWeather = function(){
			var yahooApi = "http://query.yahooapis.com/v1/public/yql?q=select%20item%20from%20weather.forecast%20where%20woeid=" + $scope.weatherWOEID + "%20and%20u=%27c%27&format=json";
			// var yahooApi = 'https://query.yahooapis.com/v1/public/yql?q=select%20item%20from%20weather.forecast%20where%20woeid%20in%20(select%20woeid%20from%20geo.places(1)%20where%20text%3D%22' + $scope.weatherCity + '%22)%20and%20u%3D\'c\'&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys';
			$http.get(yahooApi)
				.then(function(response){
					weather = $scope.weather = response.data.query.results.channel.item;
				});
		};

		$scope.removeChart = function(i) {
			$scope.dayData.splice(i, 1);
			if($scope.dayData.length == 0){
				$scope.dayData.push({
					enabledDays: {0: true, 1:true, 2:true, 3:true, 4:true, 5:true, 6:true},
					values: prepareDataForBars([{time:0, temp: 18}])
				});
			}
		};

		$scope.getStatus = function() {
			$scope.deregisterWatch();
			$http.get(deviceUrl + '/get_status')
				.then(function(response) {
					$scope.status = response.data;
					$scope.registerWatch();
					$scope.activeTab = $scope.status.temperatureMode;
					$scope.weatherWOEID = $scope.status.weatherWoeid;
					$scope.getWeather();
					$scope.getCity();
				})
				.catch(deviceUnreachableHandler);
		};

		$scope.refresh = function() {
			$http.get(deviceUrl + '/get_auto_temp')
				.then(function(response) {					
					dayData = $scope.dayData = parseResponse(response.data);

					$scope.getStatus();
				})
				.catch(deviceUnreachableHandler);
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
			}, 200);
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
				d.enabledDays[day] = Object.is(chartData, d);
			});
		};

		$scope.registerWatch = function(){
			$scope.manualTempWatcher = $scope.$watch('status.manual_temp', $scope.saveManualTempOnChange);
			$scope.weatherWoeidWatcher = $scope.$watch('weatherWOEID', $scope.weatherWoeidChange)
		};
		$scope.deregisterWatch = function(){
			if($scope.manualTempWatcher) $scope.manualTempWatcher();
			if($scope.weatherWoeidWatcher) $scope.weatherWoeidWatcher();
		};

		$scope.saveWoeid = function(woeid){
			$http.post(deviceUrl + '/command', {
				weatherWoeid: woeid
			});
			console.log('woeid saved ', woeid);
		}

		$scope.weatherWoeidChange = function(newVal, oldVal){			
			if(!newVal || newVal == oldVal) return;
			$scope.getWeather();
			$scope.saveWoeid(newVal);
		};

		$scope.refresh();
	}]);