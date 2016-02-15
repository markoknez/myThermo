angular.module('thermo', [])
	.controller('test', ['$scope', '$timeout', '$http', function($scope, $timeout, $http) {
		$scope.status = {};
		$scope.temperatures = [];

		$scope.$watch('deleteClick', function(newVal, old) {
			deleteClick = newVal;
		});

		$scope.save = function() {
			$http.post('http://mrostudios.duckdns.org:23231/set_auto_temp', generateResponse(getDataFromBars(states)))
				.then(function() {
					$scope.saved = true;
					$timeout(function() {
						$scope.saved = false;
					}, 2000);
				});
		};

		$scope.charts = [];

		$scope.addChart = function(data){
			$scope.charts.push(new chart(data));			
		};

		$scope.getStatus = function(){
			$http.get('http://mrostudios.duckdns.org:23231/get_status')
				.then(function (response){
					$scope.status = response.data;
					$scope.charts[0].setCurrentTemp($scope.status.temp);
				});
		};

		$scope.refresh = function() {
			$http.get('http://mrostudios.duckdns.org:23231/get_auto_temp')
				.then(function(response) {
					allStates = parseResponse(response.data);
					dayStates = getStatesByDay(allStates);

					data = dayStates[0].values;
					
					$scope.addChart(data);
					$scope.getStatus();
				});
		};
		
		$scope.setOffset = function(offset){
			$http.post("http://mrostudios.duckdns.org:23231/command", {
				time_offset: offset
			});
		};
		$scope.setManualTemp = function(temp){
			$http.post("http://mrostudios.duckdns.org:23231/command", {
				manual_temp: d3.format('.2f')(temp)
			});
		};

		$scope.days = ['Mon','Tue','Wed','Thu','Fri','Sat','Sun'];
		$scope.enabledDays = {};
		$scope.toggleDay = function(day){
			if($scope.dayEnabled(day)) 
				$scope.enabledDays[day] = false;
			else 
				$scope.enabledDays[day] = true;
		};
		$scope.dayEnabled = function(day){
			return $scope.enabledDays[day] === true;
		};


		$scope.toggleMode = function(){
			if($scope.status.temperatureMode == 'manual'){
				$http.get("http://mrostudios.duckdns.org:23231/set_mode/auto");
				$scope.status.temperatureMode = 'automatic';
				return;
			}			

			$http.get('http://mrostudios.duckdns.org:23231/set_mode/manual');
			$scope.status.temperatureMode = 'manual';
		};

		$scope.refresh();		
	}]);