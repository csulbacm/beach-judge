$(document).ready(function(){
	var _groupList = $('#grouplist');
	var _userList = $('#userlist');
	var _createUserForm = $('#create-user-form');

	var _groupList = $('#grouplist');
	jUserGroupSelect = function() {
		console.log(_groupList[0].selectedIndex);
	}
	jUserUserSelect = function() {
		console.log(_userList[0].selectedIndex);
	}

	judge.msgHandler['CU'] = function(msg) {
		if (typeof msg.err != 'undefined') {
			var errBox = _createUserForm.find('.form-error');
			if (msg.err === 'I') {
				errBox.html('Error: Form data is invalid.');
				errBox.show();
			} else if (msg.err === 'P') {
				errBox.html('Error: The passwords do not match.');
				errBox.show();
			} else if (msg.err === 'N') {
				errBox.html('Error: A user exists with that name.');
				errBox.show();
			}
		} else {
			_userList.children().each(function() {
				if (this.firstChild.innerHTML.toLowerCase().localeCompare(msg.n.toLowerCase()) > 0) {
					$(this).before('<option id=\'' + msg.i
						+ '\'>' + msg.n + '</option>');
					return false;
				}
				if (this.parentNode.lastChild === this) {
					$(this).before('<option id=\'' + msg.i
						+ '\'>' + msg.n + '</option>');
					return false;
				}
			});
		}
	};

	judge.msgHandler['UL'] = function(msg) {
		var h = '<option>Select User</option>';
		//TODO: Use User ID
		for (var a = 0; a < msg.users.length; ++a) {
			h += '<option id=\'' + msg.users[a].i
						+ '\'>' + msg.users[a].n + '</option>';
		}
		_userList.html(h);
	};

	_createUserForm.submit(function(evt) {
		$(this).find('.form-error').hide();
		judgeQueue('CU ' + $(this).serialize());
		return false;
	});


});

