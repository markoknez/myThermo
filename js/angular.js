deviceUrl = 'http://192.168.64.110'

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

				$scope.$watch('enabledDays', function(newVal, oldVal){
					var weekDay = new Date().getDay() - 1;
					if($scope.enabledDays[weekDay] == true)
						$scope.chart.addCurrentTime();
					else
						$scope.chart.removeCurrentTime();
				}, true);
			}
		};
	})
	.directive('arcPicker', ['$timeout', function($timeout){
		return {
			restrict: 'E',
			template: '<div class="arc-placeholder"></div>',
			scope: {value: '=', isPicker: '='},
			link: function($scope, $element, $attributes){
				$scope.arc = new arcPicker($element[0], $scope.isPicker, $scope);
				$scope.$watch('value',function(newVal, oldVal){
					if(!newVal || newVal == oldVal) return;

					$scope.arc.setValue(newVal);
				});

				$scope.$on('valueChanged', function(event, newVal){
						(newVal);
					$timeout(function (){
						$scope.value = newVal;
					});
				});
			}
		};
	}])
	.controller('testController', ['$scope', '$timeout', '$http', function($scope, $timeout, $http) {
		$scope.status = {};
		$scope.temperatures = [];

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

		$scope.removeChart = function(i) {
			$scope.dayData.splice(i, 1);
		};

		$scope.getStatus = function() {
			$http.get(deviceUrl + '/get_status')
				.then(function(response) {
					$scope.status = response.data;
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

		$scope.setManualTemp = function(temp) {
			$http.post(deviceUrl + '/command', {
				manual_temp: d3.format('.2f')(temp)
			});
		};

		$scope.toggleMode = function() {
			if ($scope.status.temperatureMode == 'manual') {
				$http.get(deviceUrl + '/set_mode/auto');
				$scope.status.temperatureMode = 'automatic';
				return;
			}

			$http.get(deviceUrl + '/set_mode/manual');
			$scope.status.temperatureMode = 'manual';
		};

		$scope.toggleDay = function(chartData, day) {
			$scope.dayData.forEach(function(d) {
				d.enabledDays[day] = chartData == d;
			});
		};

		$scope.refresh();
	}]);