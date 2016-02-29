angular.module('thermo')
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
					if($scope.enabledDays[weekDay] == true){
						$scope.chart.addCurrentTime();
						console.log('addCurrentTime');
					}
					else{
						$scope.chart.removeCurrentTime();
						console.log('removeCurrentTime');
					}
				}, true);
			}
		};
	});