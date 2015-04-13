if (!Date.now) {
	Date.now = function now() {
		return new Date().getTime();
	};
}

if (window.location.protocol != 'https:')
	window.location.href = 'https:' + window.location.href.substring(window.location.protocol.length);
function setCookie(cname, cvalue, exdays) {
	var d = new Date();
	d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
	var expires = 'expires=' + d.toUTCString();
	document.cookie = cname + '=' + cvalue + ';' + expires;
}
function deleteCookie(cname) {
	document.cookie = cname + '=; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
}

// From libwebsockets text.html example
function get_appropriate_ws_url()
{
	var pcol;
	var u = document.URL;

	/*
	 * We open the websocket encrypted if this page came on an
	 * https:// url itself, otherwise unencrypted
	 */

	if (u.substring(0, 5) == 'https') {
		pcol = 'wss://';
		u = u.substr(8);
	} else {
		pcol = 'ws://';
		if (u.substring(0, 4) == 'http')
			u = u.substr(7);
	}

	u = u.split('/');

	/* + '/xxx' bit is for IE10 workaround */

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

var wsUrl = get_appropriate_ws_url();
function judgeConnect() {
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
				window.location = window.location;
			}
		}
	});
}

function judgePopulate() {
	judgeQueue('POP:');
}

judgeConnect();

//- Navigation -
var judgeLastState;
function onNavigate(stateObj) {
	var container;
	if (judgeLastState) {
		container = judgeLastState.nav.substr(1);
		if (container.length)
			$('#' + container).hide();
		else
			$('#root').hide();
	}
	container = stateObj.nav.substr(1);
	if (container.length)
		$('#' + container).show();
	else
		$('#root').show();
	judgeLastState = stateObj;
	if (judgeDebug )
		console.log('Nav:' + JSON.stringify(stateObj));

	if (stateObj.nav === '/teams') {
		judgeQueue('TL:');
	}
}
window.onpopstate = function(evt) {
	onNavigate(evt.state);
};
function nav(target) {
	var stateObj = { nav: target };
	history.pushState(stateObj, 'beachJudge', stateObj.nav);
	onNavigate(stateObj);
}

//- Initialization -
$(document).ready(function(){
	judgePopulate();
	var stateObj = { nav: document.location.pathname };
	history.replaceState(stateObj, 'beachJudge', stateObj.nav);
	onNavigate(stateObj);
});

//- Cleanup -
$(window).on('beforeunload', function() {
	judge.ws.close();
});
