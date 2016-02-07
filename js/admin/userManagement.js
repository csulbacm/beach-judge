$(document).ready(function(){
	var _userList = $('#userlist');
	var _userCreateForm = $('#create-user-form');
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

	judge.msgHandler['CU'] = function(msg) {
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

	_userCreateForm.submit(function(evt) {
		$(this).find('.form-error').hide();
		judgeQueue('CU ' + $(this).serialize());
		return false;
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
		// Hide placeholder
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
		_userGroupCreateForm[0].reset();
	});
	$('#jfug-cr-cr').click(function() {
		if (confirm("Are you sure you want to create this user group?") == 0)
			return;
		_userGroupCreateFormError.parent().hide();
		judgeQueue('CUG ' + _userGroupCreateForm.serialize());
	});

	// Editing
	$('#jfug-ed-ca').click(function() {
		_userGroupEditFormError.parent().hide();
		_userGroupEditForm[0].reset();
		nav('/usergroups');
	});
	$('#jfug-ed-cl').click(function() {
		_userGroupEditForm[0].reset();
	});
	$('#jfug-ed-up').click(function() {
		if (confirm("Are you sure you want to update this user group?") == 0)
			return;
		_userGroupEditFormError.parent().hide();
		console.log('Update');
	});
	$('#jfug-ed-dl').click(function() {
		if (confirm("Are you sure you want to delete this user group?") == 0)
			return;
		console.log('DUG ' + $('#jfug-ed-i').serialize());
		judgeQueue('DUG ' + $('#jfug-ed-i').serialize());
	});

	judge.msgHandler['CUG'] = function(msg) {
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
	judge.msgHandler['DUG'] = function(msg) {
		if (typeof msg.err != 'undefined') {
			var errBox = _userGroupEditFormError;
			var parent = errBox.parent();
			if (msg.err === 'I') {
				errBox.html('Error: Form data is invalid.');
				parent.show();
			} else if (msg.err === 'D') {
				errBox.html('Error: A usergroup does not exist with that id.');
				parent.show();
			}
		} else {
			_userGroupEditForm[0].reset();
			nav('/usergroups');
		}
	};
});

