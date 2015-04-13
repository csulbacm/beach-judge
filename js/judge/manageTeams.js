$(document).ready(function(){
	judge.msgHandler['CT'] = function(evt) {
		console.log('CT: ' + JSON.stringify(evt));
	};

	$("#create-team-form").submit(function(evt) {
		judgeQueue('CT ' + $(this).serialize());
		return false;
	});
});
