$(document).ready(function() {
//TODO: Redo error messages

var _problemSetList = $('#problemset-list');
var _problemSetCreateForm = $('#jfps-cr');
var _problemSetCreateFormError = $('#jfps-cr-er');
var _problemSetEditForm = $('#jfps-ed');
var _problemSetEditFormError = $('#jfps-ed-er');
var _problemList = $('#problem-list');
var _problemCreateForm = $('#jfp-cr');
var _problemCreateFormError = $('#jfp-cr-er');
var _problemEditForm = $('#jfp-ed');
var _problemEditFormError = $('#jfp-ed-er');

var _problemSetHover = $('#problemset-hover');
var _problemSetHoverTarget = '';
var _problemSetHoverTimeout = '';
var _problemHover = $('#problem-hover');
var _problemHoverTarget = '';
var _problemHoverTimeout = '';

var _formatDate = function(d)
{
	var year = d.getFullYear();
	var month = '0' + (1 + d.getMonth());
	var day = '0' + d.getDate();
	var hours = '0' + d.getHours();
	var minutes = '0' + d.getMinutes();
	var seconds = '0' + d.getSeconds();
	return year
		+ '-' + month.substr(-2)
		+ '-' + day.substr(-2)
		+ ' ' + hours.substr(-2) + ':' + minutes.substr(-2) + ':' + seconds.substr(-2);
}


//------------- Problems ----------------

// Navigation
judge.onEnter['problem-create'] = function(state) {
	$('#jfp-cr-s').val(state.args[0]);
};
judge.onLeave['problem-create'] = function(state) {
	_problemCreateFormError.parent().hide();
	_problemCreateForm[0].reset();
};
judge.onEnter['problem-edit'] = function(state) {
	judgeQueue('PI s=' + state.args[0] + '&i=' + state.args[1]);
	//TODO: Confirm this redudancy is necessary
	$('#jfp-ed-s').val(state.args[0]);
	$('#jfp-ed-i').val(state.args[1]);
};
judge.onLeave['problem-edit'] = function(state) {
	_problemEditFormError.parent().hide();
	_problemEditForm[0].reset();
};

// Effects
//TODO: Make this work for onClick for mobile
$('#problem-list').on('mouseenter', 'a', function() {
	clearTimeout(_problemHoverTimeout);
	var rect = this.getBoundingClientRect();
	_problemHoverTarget = $(this);
	_problemHover.css({top: rect.top, left: rect.left, width: rect.right - rect.left});
	_problemHover.show();
}).on('mouseleave', 'a', function() {
	_problemHoverTimeout = setTimeout(function() {
		_problemHover.hide();
	}, 100);
});


// Messages
judge.onMsg['PC'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _problemCreateFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'N') {
			errBox.html('Error: A problem exists with that name.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A problem set does not exist with that set id.');
		}
	} else {
		_problemCreateForm[0].reset();
		nav('/problemset/edit/' + $('#jfp-cr-s').val());
	}
};
judge.onMsg['PD'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _problemEditFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A problem does not exist with that id.');
		}
	} else {
		_problemEditForm[0].reset();
		nav('/problemset/edit/' + $('#jfp-ed-s').val());
	}
};
judge.onMsg['PI'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		//TODO: Determine if any other error handling is necessary
		nav('/problemsets');
	} else {
		$('#jfp-ed-s').val(msg.s);
		$('#jfp-ed-i').val(msg.i);
		$('#jfp-ed-n').val(msg.n);
	}
};
judge.onMsg['PL'] = function(msg) {
	$('#problemset-edit .placeholder').hide();
	if (msg.data.length == 0) {
		_problemList.html("<p>There are no problems in this set.</p>");
		return;
	}
	var h = '';
	for (var a = 0; a < msg.data.length; ++a) {
		h += '<li><a href="/problem/edit/' + $('#jfps-ed-i').val() + '/' + msg.data[a].i + '" i="' + msg.data[a].i
			+ '">' + msg.data[a].n
			+ '</a></li>';
	}
	_problemList.html(h);
};
judge.onMsg['PU'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _problemEditFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'S') {
			errBox.html('Error: A problem set does not exist with that id.');
		} else if (msg.err === 'N') {
			errBox.html('Error: A problem exists with that name.');
		} else if (msg.err === 'C') {
			errBox.html('Error: There are no changes to be made.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A problem does not exist with that id.');
		}
	} else {
		_problemEditForm[0].reset();
		nav('/problemset/edit/' + $('#jfp-ed-s').val());
	}
};

// Creation
$('#jfp-cr-ca').click(function() {
	nav('/problemset-edit/' + $('#jfp-cr-s').val());
});
$('#jfp-cr-cl').click(function() {
	_problemCreateFormError.parent().hide();
	_problemCreateForm[0].reset();
});
$('#jfp-cr-cr').click(function() {
	_problemCreateFormError.parent().hide();
	if (confirm("Are you sure you want to create this problem?") == 0)
		return;
	judgeQueue('PC ' + _problemCreateForm.serialize());
});

// Editing
$('#jfp-ed-ca').click(function() {
	nav('/problemset-edit/' + $('#jfp-ed-s').val());
});
$('#jfp-ed-cl').click(function() {
	_problemEditFormError.parent().hide();
	_problemEditForm[0].reset();
});
$('#jfp-ed-up').click(function() {
	_problemEditFormError.parent().hide();
	if (confirm("Are you sure you want to update this problem?") == 0)
		return;
	judgeQueue('PU ' + _problemEditForm.serialize());
});
$('#jfp-ed-dl').click(function() {
	_problemEditFormError.parent().hide();
	if (confirm("Are you sure you want to delete this problem?") == 0)
		return;
	judgeQueue('PD ' + $('#jfp-ed-s').serialize() + '&' + $('#jfp-ed-i').serialize());
});


//----------- Problem Sets --------------

// Navigation
judge.onEnter['problemsets'] = function(state) {
	_problemSetList.html('');
	$('#problemsets .placeholder').show(1, function() {
		var t = this;
		setTimeout(function() {
			$('#problemset-loading').css(t.getBoundingClientRect());
		}, 0);
		judgeQueue('PSL');
	});
};
judge.onEnter['problemset-create'] = function(state) {
	$('#jfps-cr-s').val(0);
	var d = new Date();
	var m = Math.ceil(d.getMinutes()/30 + d.getSeconds()/1800)*30;
	d.setSeconds(0);
	if (m == 60) {
		d.setHours(d.getHours() + 1);
		d.setMinutes(0);
	} else
		d.setMinutes(m);
	$('#jfps-cr-t').val(_formatDate(d));
	$('#jfps-cr-d').val(7200);
	$('#jfps-cr-o').val(d.getTimezoneOffset()/-60);
};
judge.onLeave['problemset-create'] = function(state) {
	_problemSetCreateFormError.parent().hide();
	_problemSetCreateForm[0].reset();
};
judge.onEnter['problemset-edit'] = function(state) {
	judgeQueue('PSI i=' + state.args[0]);
	//TODO: Confirm this redudancy is necessary
	$('#jfps-ed-i').val(state.args[0]);
	$('#jfps-ed-o').val((new Date()).getTimezoneOffset()/-60);
	_problemList.html('');
	$('#problem-create-lnk').prop('href', '/problem/create/' + state.args[0]);
	$('#problemset-edit .placeholder').show(1, function() {
		var t = this;
		setTimeout(function() {
			$('#problem-loading').css(t.getBoundingClientRect());
		}, 0);
		judgeQueue('PL i=' + state.args[0]);
	});
};
judge.onLeave['problemset-edit'] = function(state) {
	_problemSetEditFormError.parent().hide();
	_problemSetEditForm[0].reset();
};

// Effects
//TODO: Make this work for onClick for mobile
$('#problemset-list').on('mouseenter', 'a', function() {
	clearTimeout(_problemSetHoverTimeout);
	var rect = this.getBoundingClientRect();
	_problemSetHoverTarget = $(this);
	_problemSetHover.css({top: rect.top, left: rect.left, width: rect.right - rect.left});
	_problemSetHover.show();
}).on('mouseleave', 'a', function() {
	_problemSetHoverTimeout = setTimeout(function() {
		_problemSetHover.hide();
	}, 100);
});

// Messages
judge.onMsg['PSC'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _problemSetCreateFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'N') {
			errBox.html('Error: Invalid problem set name.');
		} else if (msg.err === 'S') {
			errBox.html('Error: Invalid problem set status.');
		} else if (msg.err === 'D') {
			errBox.html('Error: Invalid problem set duration.');
		} else if (msg.err === 'O') {
			errBox.html('Error: Invalid problem set offset.');
		}
	} else {
		_problemSetCreateForm[0].reset();
		nav('/problemsets');
	}
};
judge.onMsg['PSD'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _problemSetEditFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A problem set does not exist with that id.');
		}
	} else {
		_problemSetEditForm[0].reset();
		nav('/problemsets');
	}
};
judge.onMsg['PSI'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		//TODO: Determine if any other error handling is necessary
		nav('/problemsets');
	} else {
		$('#jfps-ed-i').val(msg.i);
		$('#jfps-ed-n').val(msg.n);
		$('#jfps-ed-d').val(msg.d);
		$('#jfps-ed-t').val(_formatDate(new Date(msg.t * 1000)));
		$('#jfps-ed-s').val(msg.s);
	}
};
judge.onMsg['PSL'] = function(msg) {
	$('#problemsets .placeholder').hide();
	if (msg.data.length == 0) {
		_problemSetList.html("<p>There are no problem sets.</p>");
		return;
	}
	var h = '';
	for (var a = 0; a < msg.data.length; ++a) {
		//TODO: Optimize this as much as possible
		//TODO: Show duration and status
		var date = new Date(msg.data[a].t * 1000);
		h += '<li><a href="/problemset/edit/' + msg.data[a].i + '" i="' + msg.data[a].i
			+ '">' + msg.data[a].n;
		h += ' <span class="dt">' + _formatDate(date) + '</span>';
		h += '</a></li>';
	}
	_problemSetList.html(h);
};
judge.onMsg['PSU'] = function(msg) {
	if (typeof msg.err != 'undefined') {
		var errBox = _problemSetEditFormError;
		errBox.parent().show();
		if (msg.err === 'I') {
			errBox.html('Error: Form data is invalid.');
		} else if (msg.err === 'N') {
			errBox.html('Error: Invalid problem set name.');
		} else if (msg.err === 'D') {
			errBox.html('Error: Invalid problem set duration.');
		} else if (msg.err === 'T') {
			errBox.html('Error: Invalid problem set start time.');
		} else if (msg.err === 'A') {
			errBox.html('Error: Invalid problem set status.');
		} else if (msg.err === 'S') {
			errBox.html('Error: There are no changes to be made.');
		} else if (msg.err === 'U') {
			errBox.html('Error: A problem set does not exist with that id.');
		}
	} else {
		_problemSetEditForm[0].reset();
		nav('/problemsets');
	}
};

// Creation
$('#jfps-cr-ca').click(function() {
	nav('/problemsets');
});
$('#jfps-cr-cl').click(function() {
	_problemSetCreateFormError.parent().hide();
	_problemSetCreateForm[0].reset();
});
$('#jfps-cr-cr').click(function() {
	_problemSetCreateFormError.parent().hide();
	if (confirm("Are you sure you want to create this problem set?") == 0)
		return;
	judgeQueue('PSC ' + _problemSetCreateForm.serialize());
});

// Editing
$('#jfps-ed-ca').click(function() {
	nav('/problemsets');
});
$('#jfps-ed-cl').click(function() {
	_problemSetEditFormError.parent().hide();
	_problemSetEditForm[0].reset();
});
$('#jfps-ed-up').click(function() {
	_problemSetEditFormError.parent().hide();
	if (confirm("Are you sure you want to update this problem set?") == 0)
		return;
	judgeQueue('PSU ' + _problemSetEditForm.serialize());
});
$('#jfps-ed-dl').click(function() {
	_problemSetEditFormError.parent().hide();
	if (confirm("Are you sure you want to delete this problem set?") == 0)
		return;
	judgeQueue('PSD ' + $('#jfps-ed-i').serialize());
});

});
