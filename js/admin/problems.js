$(document).ready(function() {

var _problemSetList = $('#problemset-list');

var _problemSetHover = $('#problemset-hover');
var _problemSetHoverTarget = '';
var _problemSetHoverTimeout = '';

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
