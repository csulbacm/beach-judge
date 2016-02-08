$(document).ready(function(){
	var _userList = $('#user-list');
	var _userCreateForm = $('#jfu-cr');
	var _userCreateFormError = $('#jfu-cr-er');
	var _userEditForm = $('#jfu-ed');
	var _userEditFormError = $('#jfu-ed-er');
	var _userGroupCreateForm = $('#jfug-cr');
	var _userGroupCreateFormError = $('#jfug-cr-er');
	var _userGroupEditForm = $('#jfug-ed');
	var _userGroupEditFormError = $('#jfug-ed-er');

	var _groupList = $('#usergroup-list');
	jUserGroupSelect = function() {
		console.log(_groupList[0].selectedIndex);
	}
	jUserUserSelect = function() {
		console.log(_userList[0].selectedIndex);
	}

	judge.msgHandler['UC'] = function(msg) {
		if (typeof msg.err != 'undefined') {
			var errBox = _userCreateForm.find('.form-error');
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
			} else if (msg.err === 'U') {
				errBox.html('Error: A usergroup does not exist with that group id.');
				parent.show();
			}
		} else {
			_userCreateForm[0].reset();
			window.history.back();
			//TODO: Go back to usergroup edit
		}
	};

	judge.msgHandler['UI'] = function(msg) {
		//$('#jfu-ed-i').val(args[0]);
	};

	judge.msgHandler['UL'] = function(msg) {
		$('#usergroup-edit .placeholder').hide();
		if (msg.users.length == 0) {
			_userList.html("<p>There are no users in this group.</p>");
			return;
		}
		var h = '';
		for (var a = 0; a < msg.users.length; ++a) {
			h += '<li><a href="/user/edit/' + msg.users[a].i + '" i="' + msg.users[a].i
				+ '">' + msg.users[a].n
				+ '</a></li>';
		}
		_userList.html(h);
	};

	_userCreateForm.submit(function(evt) {
		$(this).find('.form-error').hide();
		judgeQueue('CU ' + $(this).serialize());
		return false;
	});

	// Creation
	$('#jfu-cr-ca').click(function() {
		_userCreateFormError.parent().hide();
		_userCreateForm[0].reset();
		//TODO: Go back to usergroup page
		window.history.back();
		//nav('/usergroups');
	});
	$('#jfu-cr-cl').click(function() {
		_userCreateFormError.parent().hide();
		_userCreateForm[0].reset();
	});
	$('#jfu-cr-cr').click(function() {
		if (confirm("Are you sure you want to create this user?") == 0)
			return;
		_userCreateFormError.parent().hide();
		judgeQueue('UC ' + _userCreateForm.serialize());
	});


	//------------ User Groups --------------
	/*$('#usergroup-list li').click(function() {
		console.log(this.getAttribute("i"));
		return false;
	});
	*/
	//TODO: Make this work for onClick for mobile
	var jUserGroupMouseTimeout = "";
	var _groupHover = $('#usergroup-hover');
	var _groupHoverTarget = "";
	$('#usergroup-list').on('mouseenter', 'a', function() {
		clearTimeout(jUserGroupMouseTimeout);
		var rect = this.getBoundingClientRect();
		//jUserGroupTarget = e.getAttribute("i");
		//_groupHover.setAttribute("href", "#" + e.getAttribute("i"));
		_groupHoverTarget = $(this);
	//	_groupHoverTarget.addClass('hover');
		_groupHover.css({top: rect.top, left: rect.left, width: rect.right - rect.left});
		_groupHover.show();
	}).on('mouseleave', 'a', function() {
	//	_groupHoverTarget.removeClass('hover');
		jUserGroupMouseTimeout = setTimeout(function() {
			_groupHover.hide();
		}, 100);
	});

	var _userGroupList = $('#usergroup-list');
	judge.msgHandler['UGL'] = function(msg) {
		$('#usergroups .placeholder').hide();
		if (msg.usergroups.length == 0) {
			_groupList.html("<p>There are no user groups.</p>");
			return;
		}
		var h = '';
		for (var a = 0; a < msg.usergroups.length; ++a) {
			h += '<li><a href="/usergroup/edit/' + msg.usergroups[a].i + '" i="' + msg.usergroups[a].i
				+ '">' + msg.usergroups[a].n;
			if (msg.usergroups[a].a == false)
				h += '<span class="ia"> - Inactive</span>';
			h += '</a></li>';
		}
		_userGroupList.html(h);
	};

	// Creation
	$('#jfug-cr-ca').click(function() {
		_userGroupCreateFormError.parent().hide();
		_userGroupCreateForm[0].reset();
		nav('/usergroups');
	});
	$('#jfug-cr-cl').click(function() {
		_userGroupCreateFormError.parent().hide();
		_userGroupCreateForm[0].reset();
	});
	$('#jfug-cr-cr').click(function() {
		if (confirm("Are you sure you want to create this user group?") == 0)
			return;
		_userGroupCreateFormError.parent().hide();
		judgeQueue('UGC ' + _userGroupCreateForm.serialize());
	});

	// Editing
	$('#jfug-ed-ca').click(function() {
		_userGroupEditFormError.parent().hide();
		_userGroupEditForm[0].reset();
		nav('/usergroups');
	});
	$('#jfug-ed-cl').click(function() {
		_userGroupEditFormError.parent().hide();
		_userGroupEditForm[0].reset();
	});
	$('#jfug-ed-up').click(function() {
		if (confirm("Are you sure you want to update this user group?") == 0)
			return;
		_userGroupEditFormError.parent().hide();
		judgeQueue('UGU ' + _userGroupEditForm.serialize());
	});
	$('#jfug-ed-dl').click(function() {
		if (confirm("Are you sure you want to delete this user group?") == 0)
			return;
		judgeQueue('UGD ' + $('#jfug-ed-i').serialize());
	});

	judge.msgHandler['UGC'] = function(msg) {
		if (typeof msg.err != 'undefined') {
			var errBox = _userGroupCreateFormError;
			var parent = errBox.parent();
			if (msg.err === 'I') {
				errBox.html('Error: Form data is invalid.');
				parent.show();
			} else if (msg.err === 'N') {
				errBox.html('Error: A usergroup exists with that name.');
				parent.show();
			}
		} else {
			_userGroupCreateForm[0].reset();
			nav('/usergroups');
			return;
			//TODO: Revisit this or remove
			/*
			_userGroupList.children().each(function() {
				if (this.firstChild.innerHTML.toLowerCase().localeCompare(msg.n.toLowerCase()) > 0) {
					$(this).before('<li><a href="/usergroup/edit/'
						+ msg.i
						+ '" i="' + msg.i
						+ '">' + msg.n + '</a></li>');
					return false;
				}
				if (this.parentNode.lastChild === this) {
					$(this).before('<li><a href="/usergroup/edit/'
						+ msg.i
						+ '" i="' + msg.i
						+ '">' + msg.n + '</a></li>');
					return false;
				}
			});
			*/
		}
	};
	judge.msgHandler['UGD'] = function(msg) {
		if (typeof msg.err != 'undefined') {
			var errBox = _userGroupEditFormError;
			var parent = errBox.parent();
			if (msg.err === 'I') {
				errBox.html('Error: Form data is invalid.');
				parent.show();
			} else if (msg.err === 'G') {
				errBox.html('Error: You cannot delete the global user group.');
				parent.show();
			} else if (msg.err === 'U') {
				errBox.html('Error: A usergroup does not exist with that id.');
				parent.show();
			}
		} else {
			_userGroupEditForm[0].reset();
			nav('/usergroups');
		}
	};
	judge.msgHandler['UGI'] = function(msg) {
		if (typeof msg.err != 'undefined') {
			//TODO: Determine if any other error handling is necessary
			nav('/usergroups');
		} else {
			$('#jfug-ed-i').val(msg.i);
			$('#jfug-ed-n').val(msg.n);
			$('#jfug-ed-a').prop('checked', msg.a == 1);
		}
	};
	judge.msgHandler['UGU'] = function(msg) {
		if (typeof msg.err != 'undefined') {
			var errBox = _userGroupEditFormError;
			var parent = errBox.parent();
			if (msg.err === 'I') {
				errBox.html('Error: Form data is invalid.');
				parent.show();
			} else if (msg.err === 'G') {
				errBox.html('Error: You cannot alter the global user group.');
				parent.show();
			} else if (msg.err === 'N') {
				errBox.html('Error: A usergroup exists with that name.');
				parent.show();
			} else if (msg.err === 'S') {
				errBox.html('Error: There are no changes to be made.');
				parent.show();
			} else if (msg.err === 'U') {
				errBox.html('Error: A usergroup exists with that id.');
				parent.show();
			}
		} else {
			_userGroupEditForm[0].reset();
			nav('/usergroups');
		}
	};
});

