$(document).ready(function(){
	var _teamList = $('#teamlist');
	var _createTeamForm = $("#create-team-form");

	judge.msgHandler['CT'] = function(msg) {
		if (typeof msg.err != 'undefined') {
			var errBox = _createTeamForm.find('.form-error');
			if (msg.err === 'I') {
				errBox.html('Error: Form data is invalid.');
				errBox.show();
			} else if (msg.err === 'P') {
				errBox.html('Error: The passwords do not match.');
				errBox.show();
			} else if (msg.err === 'N') {
				errBox.html('Error: A team exists with that name.');
				errBox.show();
			}
		} else {
			_teamList.children().each(function() {
				if (this.firstChild.innerText.toLowerCase().localeCompare(msg.n.toLowerCase()) > 0) {
					$(this).before('<li><a href="javascript:selectTeam(\'' + msg.i + '\');">' + msg.n + '</a></li>');
					return false;
				}
				if (this.parentNode.lastChild === this) {
					$(this).after('<li><a href="javascript:selectTeam(\'' + msg.i + '\');">' + msg.n + '</a></li>');
					return false;
				}
			});
		}
	};

	judge.msgHandler['TL'] = function(msg) {
		var h = '';
		//TODO: Use team ID
		for (var a = 0; a < msg.teams.length; ++a)
			h += '<li><a href="javascript:selectTeam(\'' + msg.teams[a].i + '\');">' + msg.teams[a].n + '</a></li>';
		_teamList.html(h);
	};

	_createTeamForm.submit(function(evt) {
		$(this).find('.form-error').hide();
		judgeQueue('CT ' + $(this).serialize());
		return false;
	});
});

function selectTeam(id) {
	console.log(id);
}
