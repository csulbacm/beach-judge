$(document).ready(function(){
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
			var parent = errBox.parent();
			if (msg.err === 'I') {
				errBox.html('Error: Form data is invalid.');
				parent.show();
			} else if (msg.err === 'P') {
				errBox.html('Error: The passwords do not match.');
				parent.show();
			} else if (msg.err === 'N') {
				errBox.html('Error: A user exists with that name.');
				parent.show();
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

	//------------ User Groups --------------
	
	// Creation
	$('#jfug-cr-ca').click(function() { this.parentNode.parentNode.reset(); nav('/usergroups'); });
	$('#jfug-cr-cl').click(function() { this.parentNode.parentNode.reset(); });
	$('#jfug-cr-cr').click(function() {
		console.log("Create");
		//	judgeGroupCreate(form)
	});

	var _groupList = $('#grouplist');
	judge.msgHandler['UGL'] = function(msg) {
		if (msg.usergroups.length == 0) {
			_groupList.html("<p>There are no user groups.</p>");
			return;
		}
		var h = '';
		for (var a = 0; a < msg.usergroups.length; ++a) {
			h += '<li><a href="/usergroup/'
				+ msg.usergroups[a].i
				+ '" onmouseenter="jUserGroupMouseEnter(this)"'
				+ ' onmouseleave="jUserGroupMouseLeave()"'
				+ 'i="' + msg.usergroups[a].i
				+ '">' + msg.usergroups[a].n + '</a></li>';
		}
		_groupList.html(h);
	};

	//TODO: Make this work for onClick for mobile
	var jUserGroupMouseTimeout = "";
	var _groupHover = $('#usergroup-hover');
	var _groupHoverTarget = "";
	jUserGroupMouseEnter = function(e) {
		clearTimeout(jUserGroupMouseTimeout);
		var rect = e.getBoundingClientRect();
		//jUserGroupTarget = e.getAttribute("i");
		//_groupHover.setAttribute("href", "#" + e.getAttribute("i"));
		_groupHoverTarget = $(e);
	//	_groupHoverTarget.addClass('hover');
		_groupHover.css({top: rect.top, left: rect.left, width: rect.right - rect.left});
		_groupHover.show();
	}
	jUserGroupMouseLeave = function() {
	//	_groupHoverTarget.removeClass('hover');
		jUserGroupMouseTimeout = setTimeout(function() {
			_groupHover.hide();
		}, 100);
	}

});

