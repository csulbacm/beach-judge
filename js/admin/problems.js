$(document).ready(function() {
//TODO: Redo error messages

var _problemSetList = $('#problemset-list');
var _problemSetCreateForm = $('#jfps-cr');
var _problemSetCreateFormError = $('#jfps-cr-er');
var _problemSetEditForm = $('#jfps-ed');
var _problemSetEditFormError = $('#jfps-ed-er');

var _problemSetHover = $('#problemset-hover');
var _problemSetHoverTarget = '';
var _problemSetHoverTimeout = '';

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
	if (msg.problemsets.length == 0) {
		_problemSetList.html("<p>There are no problem sets.</p>");
		return;
	}
	var h = '';
	for (var a = 0; a < msg.problemsets.length; ++a) {
		//TODO: Optimize this as much as possible
		//TODO: Show duration and status
		var date = new Date(msg.problemsets[a].t * 1000);
		h += '<li><a href="/problemset/edit/' + msg.problemsets[a].i + '" i="' + msg.problemsets[a].i
			+ '">' + msg.problemsets[a].n;
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
