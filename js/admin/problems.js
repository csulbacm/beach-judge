$(document).ready(function() {

var _problemList = $('#problem-list');

var _problemHover = $('#problem-hover');
var _problemHoverTarget = '';
var _problemHoverTimeout = '';

//------------- Problems ----------------

// Navigation
judge.onEnter['problems'] = function(state) {
	console.log("HI");
	_problemList.html('');
	$('#problems .placeholder').show(1, function() {
		var t = this;
		setTimeout(function() {
			$('#problem-loading').css(t.getBoundingClientRect());
		}, 0);
		judgeQueue('PL');
	});
};
/*
judge.onEnter['user-create'] = function(state) {
	$('#jfu-cr-g').val(state.args[0]);
};
judge.onEnter['user-edit'] = function(state) {
	judgeQueue('UI i=' + state.args[0]);
	$('#jfu-ed-i').val(state.args[0]);
};
judge.onLeave['user-create'] = function(state) {
	_userCreateFormError.parent().hide();
	_userCreateForm[0].reset();
};
judge.onLeave['user-edit'] = function(state) {
	_userEditFormError.parent().hide();
	_userEditForm[0].reset();
};
*/

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

});
