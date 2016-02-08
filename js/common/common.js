if (!Date.now) {
	Date.now = function now() {
		return new Date().getTime();
	};
}

if (window.location.protocol != 'https:')
	window.location.href = 'https:' +
		window.location.href.substring(window.location.protocol.length);
function setCookie(cname, cvalue, exdays) {
	var d = new Date();
	d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
	var expires = 'expires=' + d.toUTCString();
	document.cookie = cname + '=' + cvalue + ';' + expires;
}
function deleteCookie(cname) {
	document.cookie = cname + '=; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
}

function getWSURL()
{
	var pcol;
	var u = document.URL;

	// Open secure socket if https
	if (u.substring(0, 5) == 'https') {
		pcol = 'wss://';
		u = u.substr(8);
	} else {
		pcol = 'ws://';
		if (u.substring(0, 4) == 'http')
			u = u.substr(7);
	}
	u = u.split('/');

	// Append '/xxx' for IE 10
	return pcol + u[0] + '/xxx';
}
function wsOnOpen(evt) {
	if (judgeDebug)
		console.log('judge: Connected');
	judgeProcessQueue();
}
function wsOnClose(evt) {
	if (judgeDebug)
		console.log('judge: Disconnected');
	judgeReconnect = true;
	setTimeout(judgeConnect, 1000);
}
function wsOnMessage(evt) {
	if (judgeDebug)
		console.log('judge: \'' + evt.data + '\'');
	console.log(evt.data);
	var data = JSON.parse('{' + evt.data + '}');
	if (data.msg === 'POP') {
		$('.user').html(data.name);
	} else if (typeof(judge.msgHandler[data.msg] !== 'undefined')) {
		judge.msgHandler[data.msg](data);
	}
}
function wsOnError(evt) {
	if (judgeDebug )
		console.log('judge: ERROR \'' + evt.data + '\'');
}

var judge = [];
judge.msgHandler = [];
judge.sendQueue = [];
var judgeDebug = false;
var judgeReloadOnReconnect = true;
var judgeReconnect = false;

var wsUrl = getWSURL();
function judgeConnect() {
	//TODO: Fix "The connection to wss:... was interrupted while the page was loading."
	if (typeof MozWebSocket != 'undefined') {
		judge.ws = new MozWebSocket(wsUrl,
			'judge-protocol');
	} else {
		judge.ws = new WebSocket(wsUrl,
			'judge-protocol');
	}
	
	judge.ws.url = wsUrl;
	judge.ws.onopen = wsOnOpen;
	judge.ws.onclose = wsOnClose;
	judge.ws.onmessage = wsOnMessage;
	judge.ws.onerror = wsOnError;
}
function judgeQueue(str) {
	if (judge.ws.readyState != WebSocket.OPEN)
		judge.sendQueue.push(str);
	else if (judge.sendQueue.length != 0) {
		judge.sendQueue.push(str);
		judgeProcessQueue();
	} else
		judge.ws.send(str);
}
function judgeProcessQueue() {
	if (judge.ws.readyState != WebSocket.OPEN)
		return;
	//TODO: Investigate undefined value in array
	for (var a = 0; a < judge.sendQueue.length; ++a)
		if (judge.sendQueue[a])
			judge.ws.send(judge.sendQueue[a]);
	judge.sendQueue = [];
	if (judgeReconnect && judgeReloadOnReconnect)
		window.location.reload();
}
function judgeLogout() {
	$.ajax({
		type: 'POST',
		url: '/logout',
		data: '',
		success: function(result) {
			if(result === 'ERR') {
				console.log('Logout error');
			} else {
				sessionStorage.removeItem('JUDGESESSID');
				deleteCookie('JUDGESESSID');
				window.location = window.location.origin;
			}
		}
	});
}

function judgePopulate() {
	judgeQueue('POP');
}

// Navigation
var judgeLastState;
function onNavigate(stateObj) {
	//TODO: Implement init and cleanup callbacks for pages
	//TODO: Handle 404
	var target;

	// Hide last state data
	if (judgeLastState) {
		// Don't do anything if the state hasn't changed
		if (stateObj.nav === judgeLastState.nav)
			return;

		target = judgeLastState.nav.split('/');
		if (target.length == 2)
			target = target[1];
		else if (target.length > 2) {
			var arr = target;
			var len = target.length;
			target = "";
			for (var a = 1; a < len; ++a) {
				if (isNaN(arr[a]) == false)
					break;
				if (target.length)
					target += '-' + arr[a];
				else
					target += arr[a];
			}
		} else
			target = '';
		if (target.length) {
			target = $('#' + target);
			if (target.length) {
				target.hide();
			} else
				$('#404').hide();
		} else
			$('#root').hide();
	}

	// Show new state and parse arguments
	target = stateObj.nav.split('/');
	var args = new Array();
	if (target.length == 2)
		target = target[1];
	else if (target.length > 2) {
		var arr = target;
		var len = target.length;
		target = "";
		var isArgs = false;
		for (var a = 1; a < len; ++a) {
			if (isNaN(arr[a]) == false)
				isArgs = true;
			if (isArgs) {
				args.push(arr[a]);
				continue;
			}
			if (target.length)
				target += '-' + arr[a];
			else
				target += arr[a];
		}
	} else
		target = '';
	if (target === 'usergroups') {
		//TODO: Send unix timestamp for last received data
		$('#usergroup-list').html('');
		$('#usergroups .placeholder').show(1, function() {
			var t = this;
			setTimeout(function() {
				$('#usergroup-loading').css(t.getBoundingClientRect());
			}, 0);
			judgeQueue('UGL');
		});
		//judgeQueue('UL:');
	} else if (target === 'usergroup-edit') {
		judgeQueue('UGI i=' + args[0]);
		$('#user-create-lnk').prop('href', '/user/create/' + args[0]);
		$('#usergroup-edit .placeholder').show(1, function() {
			var t = this;
			setTimeout(function() {
				$('#user-loading').css(t.getBoundingClientRect());
			}, 0);
			judgeQueue('UL i=' + args[0]);
		});
	} else if (target === 'user-edit') {
		judgeQueue('UI i=' + args[0]);
		$('#jfu-ed-i').val(args[0]);
	} else if (target === 'user-create') {
		$('#jfu-cr-g').val(args[0]);
	}
	if (target.length) {
		target = $('#' + target);
		if (target.length) {
			target.show();
		} else
			$('#404').show();
	} else
		$('#root').show();
	judgeLastState = stateObj;
	if (judgeDebug)
		console.log('Nav:' + JSON.stringify(stateObj));
}
window.onpopstate = function(evt) {
	onNavigate(evt.state);
};
function nav(target, force) {
	// Don't send request if data hasn't changed
	if (judgeLastState)
		if (judgeLastState.nav === target && force == false)
			return;
	var stateObj = { nav: target };
	history.pushState(stateObj, 'beachJudge', stateObj.nav);
	onNavigate(stateObj);
}

// Initialization
$(document).ready(function(){
	judgeConnect();
	judgePopulate();
	var stateObj = { nav: window.location.pathname };
	history.replaceState(stateObj, 'beachJudge', stateObj.nav);
	onNavigate(stateObj);

	$(document).on('click', 'a', function() {
		nav(this.getAttribute('href'));
		return false;
	});
});

// Cleanup
$(window).on('beforeunload', function() {
	judge.ws.close();
});
