angular.module('thermo')
	.directive('townSelector', function(){
		return {
			restrict: 'E',
			templateUrl: 'templates/townSelect.html',
			scope: {selectedTownName: '=', 'selectedTownWoeid': '='},
			controller: ['$scope', '$http', 'debounce', function($scope, $http, debounce){

				$scope.openDialog = function(){
					$scope.txtTown = null;
					$scope.townSelected = null;
					$scope.yahooResult = [];
				};

				$scope.getYahooTowns = function(){
					$http.get("http://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20geo.places%20where%20text%20%3D%20'"+$scope.txtTown+"'&format=json")
						.then(function (response){
							if(response.data.query.count == 0){
								$scope.yahooResult = [];
								return;
							}

							if(response.data.query.count == 1){
								$scope.yahooResult = [response.data.query.results.place];
								return;
							}

							$scope.yahooResult = response.data.query.results.place;							
						});
				};

				$scope.isSelected = function(town){
					return $scope.townSelected == town;
				};

				$scope.selectTown = function(town){
					$scope.townSelected = town;
				};

				$scope.$watch('txtTown', debounce(function(newVal, oldVal){
					if(newVal)
						$scope.getYahooTowns();
				}, 1000));

				$scope.saveSelection = function(){
					if($scope.townSelected){
						$scope.selectedTownName = $scope.townSelected.name;
						$scope.selectedTownWoeid = $scope.townSelected.woeid;
					}
				};
			}]
		};
	});