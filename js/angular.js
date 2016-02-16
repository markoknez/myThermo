angular.module('thermo', [])
	.directive('chart', function() {
		return {
			restrict: 'E',
			templateUrl: 'templates/chart.html',
			scope: {
				dayData: '=',
				status: '='
			},
			link: function($scope, $element, $attributes) {
				$scope.parentController = $scope.$parent; //TODO: remove this hack
				$scope.days = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];

				$scope.enabledDays = $scope.dayData.enabledDays;
				$scope.data = $scope.dayData.values;

				$scope.chart = new chart($($element).find('.chart-placeholder')[0], $scope.data);
				$scope.$watch('status', function(newVal, oldVal) {
					if (newVal && newVal.temp)
						$scope.chart.setCurrentTemp(newVal.temp);
				});

				$scope.dayEnabled = function(day) {
					return $scope.enabledDays[day] === true;
				};
			}
		};
	})
	.controller('testController', ['$scope', '$timeout', '$http', function($scope, $timeout, $http) {
		$scope.status = {};
		$scope.temperatures = [];

		$scope.$watch('deleteClick', function(newVal, old) {
			deleteClick = newVal;
		});

		$scope.save = function() {			
			var saveData = generateResponse(getDataForESP($scope.dayData));
			$http.post('http://mrostudios.duckdns.org:23231/set_auto_temp', saveData)
				.then(function() {
					$scope.saved = true;
					$timeout(function() {
						$scope.saved = false;
					}, 2000);
				});
		};

		$scope.charts = [];

		$scope.addChart = function(data) {
			fixData(data);
			data = prepareDataForBars(data);
			$scope.dayData.push({
				key: $scope.dayData.length + '',
				enabledDays: {},
				values: data
			});
		};

		$scope.removeChart = function(i) {
			$scope.dayData.splice(i, 1);
		};

		$scope.getStatus = function() {
			$http.get('http://mrostudios.duckdns.org:23231/get_status')
				.then(function(response) {
					$scope.status = response.data;
				});
		};

		$scope.refresh = function() {
			$http.get('http://mrostudios.duckdns.org:23231/get_auto_temp')
				.then(function(response) {
					allStates = parseResponse(response.data);
					$scope.dayData = dayData = getStatesByDay(allStates);

					$scope.getStatus();
				});
		};

		$scope.setOffset = function(offset) {
			$http.post("http://mrostudios.duckdns.org:23231/command", {
				time_offset: offset
			});
		};

		$scope.setManualTemp = function(temp) {
			$http.post("http://mrostudios.duckdns.org:23231/command", {
				manual_temp: d3.format('.2f')(temp)
			});
		};

		$scope.toggleMode = function() {
			if ($scope.status.temperatureMode == 'manual') {
				$http.get("http://mrostudios.duckdns.org:23231/set_mode/auto");
				$scope.status.temperatureMode = 'automatic';
				return;
			}

			$http.get('http://mrostudios.duckdns.org:23231/set_mode/manual');
			$scope.status.temperatureMode = 'manual';
		};

		$scope.isDayAvailable = function(chartData, day) {
			var isAvailable = true;
			$scope.dayData.forEach(function(d) {
				if (chartData != d && d.enabledDays[day]) {
					isAvailable = false;
				}
			});
			return isAvailable;
		};

		$scope.toggleDay = function(chartData, day) {
			$scope.dayData.forEach(function(d) {
				d.enabledDays[day] = chartData == d;
			});
		};

		$scope.refresh();
	}]);