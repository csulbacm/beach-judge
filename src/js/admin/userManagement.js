$(document).ready(function() {

var _userList = $('#user-list');
var _userCreateForm = $('#jfu-cr');
var _userCreateFormError = $('#jfu-cr-er');
var _userEditForm = $('#jfu-ed');
var _userEditFormError = $('#jfu-ed-er');
var _userGroupList = $('#usergroup-list');
var _userGroupCreateForm = $('#jfug-cr');
var _userGroupCreateFormError = $('#jfug-cr-er');
var _userGroupEditForm = $('#jfug-ed');
var _userGroupEditFormError = $('#jfug-ed-er');

var _userHover = $('#user-hover');
var _userHoverTarget = '';
var _userHoverTimeout = '';
var _userGroupHover = $('#usergroup-hover');
var _userGroupHoverTarget = '';
var _userGroupHoverTimeout = '';

//--------------- Users -----------------

// Navigation
judge.onEnter['user-create'] = function(state) {
	$('#jfu-cr-g').val(state.args[0]);
};
judge.onLeave['user-create'] = function(state) {
	_userCreateFormError.parent().hide();
	_userCreateForm[0].reset();
};
judge.onEnter['user-edit'] = function(state) {
	judgeQueue('UI i=' + state.args[0]);
	$('#jfu-ed-i').val(state.args[0]);
};
judge.onLeave['user-edit'] = function(state) {
	_userEditFormError.parent().hide();
	_userEditForm[0].reset();
};

// Effects
//TODO: Make this work for onClick for mobile
_userList.on('mouseenter', 'a', function() {
	clearTimeout(_userHoverTimeout);
	var rect = this.getBoundingClientRect();
	_userHoverTarget = $(this);
	_userHover.css({top: rect.top, left: rect.left, width: rect.right - rect.left});
	_userHover.show();
}).on('mouseleave', 'a', function() {
	_userHoverTimeout = setTimeout(function() {
		_userHover.hide();
	}, 100);
});

// Messages
judge.onMsg['UC'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _userCreateFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'P') {
			errBox.html('Error: The passwords do not match.');
		} else if (msg.err === 'N') {
			errBox.html('Error: A user exists with that name.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A usergroup does not exist with that group id.');
		}
	} else {
		_userCreateForm[0].reset();
		nav('/usergroup/edit/' + $('#jfu-cr-g').val());
	}
};
judge.onMsg['UD'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _userEditFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'A') {
			errBox.html('Error: You cannot delete the admin user.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A user does not exist with that id.');
		}
	} else {
		_userEditForm[0].reset();
		nav('/usergroup/edit/' + $('#jfu-ed-g').val());
	}
};
judge.onMsg['UI'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		//TODO: Determine if any other error handling is necessary
		nav('/usergroups');
	} else {
		$('#jfu-ed-g').val(msg.g);
		$('#jfu-ed-i').val(msg.i);
		$('#jfu-ed-n').val(msg.n);
		$('#jfu-ed-d').val(msg.d);
	}
};
judge.onMsg['UL'] = function(msg) {
	$('#usergroup-edit .placeholder').hide();
	if (msg.data.length == 0) {
		_userList.html("<p>There are no users in this group.</p>");
		return;
	}
	var h = '';
	for (var a = 0; a < msg.data.length; ++a) {
		h += '<li><a href="/user/edit/' + msg.data[a].i + '" i="' + msg.data[a].i
			+ '">' + msg.data[a].n
			+ '</a></li>';
	}
	_userList.html(h);
};
judge.onMsg['UU'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _userEditFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'G') {
			errBox.html('Error: A usergroup does not exist with that id.');
		} else if (msg.err === 'N') {
			errBox.html('Error: A user exists with that name.');
		} else if (msg.err === 'S') {
			errBox.html('Error: There are no changes to be made.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A user does not exist with that id.');
		}
	} else {
		_userEditForm[0].reset();
		nav('/usergroup/edit/' + $('#jfu-ed-g').val());
	}
};

// Creation
$('#jfu-cr-ca').click(function() {
	_userCreateFormError.parent().hide();
	_userCreateForm[0].reset();
	nav('/usergroup/edit/' + $('#jfu-cr-g').val());
});
$('#jfu-cr-cl').click(function() {
	_userCreateFormError.parent().hide();
	_userCreateForm[0].reset();
});
$('#jfu-cr-cr').click(function() {
	_userCreateFormError.parent().hide();
	if (confirm("Are you sure you want to create this user?") == 0)
		return;
	judgeQueue('UC ' + _userCreateForm.serialize());
});

// Editing
$('#jfu-ed-ca').click(function() {
	_userEditFormError.parent().hide();
	_userEditForm[0].reset();
	nav('/usergroup/edit/' + $('#jfu-ed-g').val());
});
$('#jfu-ed-cl').click(function() {
	_userEditFormError.parent().hide();
	_userEditForm[0].reset();
});
$('#jfu-ed-up').click(function() {
	_userEditFormError.parent().hide();
	if (confirm("Are you sure you want to update this user?") == 0)
		return;
	judgeQueue('UU ' + _userEditForm.serialize());
});
$('#jfu-ed-dl').click(function() {
	_userEditFormError.parent().hide();
	if (confirm("Are you sure you want to delete this user?") == 0)
		return;
	judgeQueue('UD ' + $('#jfu-ed-i').serialize());
});


//------------ User Groups --------------

// Navigation
judge.onEnter['usergroups'] = function(state) {
	//TODO: Send unix timestamp for last received data
	_userGroupList.html('');
	$('#usergroups .placeholder').show(1, function() {
		var t = this;
		setTimeout(function() {
			$('#usergroup-loading').css(t.getBoundingClientRect());
		}, 0);
		judgeQueue('UGL');
	});
};
judge.onEnter['usergroup-edit'] = function(state) {
	_userList.html('');
	judgeQueue('UGI i=' + state.args[0]);
	$('#user-create-lnk').prop('href', '/user/create/' + state.args[0]);
	$('#usergroup-edit .placeholder').show(1, function() {
		var t = this;
		setTimeout(function() {
			$('#user-loading').css(t.getBoundingClientRect());
		}, 0);
		judgeQueue('UL i=' + state.args[0]);
	});
};
judge.onLeave['usergroup-create'] = function(state) {
	_userGroupCreateFormError.parent().hide();
	_userGroupCreateForm[0].reset();
};
judge.onLeave['usergroup-edit'] = function(state) {
	_userGroupEditFormError.parent().hide();
	_userGroupEditForm[0].reset();
};

// Effects
//TODO: Make this work for onClick for mobile
_userGroupList.on('mouseenter', 'a', function() {
	clearTimeout(_userGroupHoverTimeout);
	var rect = this.getBoundingClientRect();
	_userGroupHoverTarget = $(this);
	_userGroupHover.css({top: rect.top, left: rect.left, width: rect.right - rect.left});
	_userGroupHover.show();
}).on('mouseleave', 'a', function() {
	_userGroupHoverTimeout = setTimeout(function() {
		_userGroupHover.hide();
	}, 100);
});

// Messages
judge.onMsg['UGC'] = function(msg) {
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
	}
};
judge.onMsg['UGD'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _userGroupEditFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'G') {
			errBox.html('Error: You cannot delete the global user group.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A usergroup does not exist with that id.');
		}
	} else {
		_userGroupEditForm[0].reset();
		nav('/usergroups');
	}
};
judge.onMsg['UGI'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		//TODO: Determine if any other error handling is necessary
		nav('/usergroups');
	} else {
		$('#jfug-ed-i').val(msg.i);
		$('#jfug-ed-n').val(msg.n);
		$('#jfug-ed-a').prop('checked', msg.a == 1);
	}
};
judge.onMsg['UGL'] = function(msg) {
	$('#usergroups .placeholder').hide();
	if (msg.data.length == 0) {
		_userGroupList.html("<p>There are no user groups.</p>");
		return;
	}
	var h = '';
	for (var a = 0; a < msg.data.length; ++a) {
		h += '<li><a href="/usergroup/edit/' + msg.data[a].i + '" i="' + msg.data[a].i
			+ '">' + msg.data[a].n;
		if (msg.data[a].a == false)
			h += ' <span class="ia">- Inactive</span>';
		h += '</a></li>';
	}
	_userGroupList.html(h);
};
judge.onMsg['UGU'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _userGroupEditFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'G') {
			errBox.html('Error: You cannot alter the global user group.');
		} else if (msg.err === 'N') {
			errBox.html('Error: A usergroup exists with that name.');
		} else if (msg.err === 'S') {
			errBox.html('Error: There are no changes to be made.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A usergroup does not exist with that id.');
		}
	} else {
		_userGroupEditForm[0].reset();
		nav('/usergroups');
	}
};

// Creation
$('#jfug-cr-ca').click(function() {
	nav('/usergroups');
});
$('#jfug-cr-cl').click(function() {
	_userGroupCreateFormError.parent().hide();
	_userGroupCreateForm[0].reset();
});
$('#jfug-cr-cr').click(function() {
	_userGroupCreateFormError.parent().hide();
	if (confirm("Are you sure you want to create this user group?") == 0)
		return;
	judgeQueue('UGC ' + _userGroupCreateForm.serialize());
});

// Editing
$('#jfug-ed-ca').click(function() {
	nav('/usergroups');
});
$('#jfug-ed-cl').click(function() {
	_userGroupEditFormError.parent().hide();
	_userGroupEditForm[0].reset();
});
$('#jfug-ed-up').click(function() {
	_userGroupEditFormError.parent().hide();
	if (confirm("Are you sure you want to update this user group?") == 0)
		return;
	judgeQueue('UGU ' + _userGroupEditForm.serialize());
});
$('#jfug-ed-dl').click(function() {
	_userGroupEditFormError.parent().hide();
	if (confirm("Are you sure you want to delete this user group?") == 0)
		return;
	judgeQueue('UGD ' + $('#jfug-ed-i').serialize());
});

});
