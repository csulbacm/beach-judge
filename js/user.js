if (!Date.now) {
	Date.now = function now() {
		return new Date().getTime();
	};
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
	if (judgeDebug )
		console.log('judge: Connected');
	judgeQueue();
}
function wsOnClose(evt) {
	if (judgeDebug )
		console.log('judge: Disconnected');
}
function wsOnMessage(evt) {
	if (judgeDebug )
		console.log('judge: \'' + evt.data + '\'');
	var data = JSON.parse('{' + evt.data + '}');
	if (data.msg === 'POP') {
		$('.user').html(data.name);
	} else if (data.msg === 'TL') {
		var h = '';
		//TODO: Use team ID
		for (var a = 0; a < data.teams.length; ++a)
			h += '<li><a href="javascript:selectTeam(' + data.teams[a] + ');">' + data.teams[a] + '</a></li>';
		$('#teamlist').html(h);
	}
}
function wsOnError(evt) {
	if (judgeDebug )
		console.log('judge: ERROR \'' + evt.data + '\'');
}


var judge = [];
var judgeDebug = false;

function judgeConnect() {
	var wsUrl = get_appropriate_ws_url();
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
	judge.sendQueue = [];
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
}

function judgePopulate() {
	judgeQueue('POP');
}

judgeConnect();

//- Navigation -
var judgeLastState;
function onNavigate(stateObj) {
	var container;
	if (judgeLastState) {
		container = judgeLastState.nav.substr(1);
		if (container != '')
			$('#' + container).hide();
		else
			$('#root').hide();
	}
	container = stateObj.nav.substr(1);
	if (container != '')
		$('#' + container).show();
	else
		$('#root').show();
	judgeLastState = stateObj;
	if (judgeDebug )
		console.log('Nav:' + JSON.stringify(stateObj));

	if (stateObj.nav === '/teams') {
		judgeQueue('TL');
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
(function ()
{
	judgePopulate();
	var stateObj = { nav: document.location.pathname };
	history.replaceState(stateObj, 'beachJudge', stateObj.nav);
	onNavigate(stateObj);
})();
