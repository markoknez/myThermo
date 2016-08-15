angular.module('thermo')
	.directive('automaticTab', function() {
		return {
			restrict: 'E',
			templateUrl: 'templates/automatic-tab.html'
		};
	});