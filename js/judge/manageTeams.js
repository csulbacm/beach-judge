$(document).ready(function(){
	judge.msgHandler['CT'] = function(evt) {
		console.log('CT: ' + JSON.stringify(evt));
	};
});

function judgeCreateTeam(form) {
	judgeQueue('CT ' + $(form).serialize());
}
