angular.module('thermo')
	.directive('manualTab', function() {
		return {
			restrict: 'E', 
			templateUrl: 'templates/manual-tab.html'
		};
	});