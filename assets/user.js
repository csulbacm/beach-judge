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
	judgePopulate();
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
	}
}
function wsOnError(evt) {
	if (judgeDebug )
		console.log('judge: ERROR \'' + evt.data + '\'');
}


var judge = [];
var judgeDebug = true;

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
}

function judgePopulate() {
	judge.ws.send('POP');
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
		console.log("Nav:" + JSON.stringify(stateObj));
}
window.onpopstate = function(evt) {
	onNavigate(evt.state);
};
function nav(target) {
	var stateObj = { nav: target };
	history.pushState(stateObj, "beachJudge", stateObj.nav);
	onNavigate(stateObj);
}

//- Initialization -
(function ()
{
	var stateObj = { nav: document.location.pathname };
	history.replaceState(stateObj, "beachJudge", stateObj.nav);
	onNavigate(stateObj);
})();
