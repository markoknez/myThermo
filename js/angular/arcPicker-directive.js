angular.module('thermo')
	.directive('arcPicker', ['$timeout', function($timeout){
		return {
			restrict: 'E',
			template: '<div class="arc-placeholder"></div>',
			scope: {value: '=', settings: '='},
			link: function($scope, $element, $attributes){
				$scope.arc = new arcPicker($element[0], $scope.settings, $scope);
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
	}]);