function question(e)
{
	var key;
	var keychar;

	if (window.event)
	   key = window.event.keyCode;
	else if (e)
	   key = e.which;
	else
	   return true;
	keychar = String.fromCharCode(key);
	keychar = keychar.toLowerCase();

	// control keys
	if ((key==null) || (key==0) || (key==8) || 
	    (key==9) || (key==13) || (key==27) )
	   return true;

	// valid characters
	else if ((("abcdefghijklmnopqrstuvwxyz0123456789 !@@#$%^&*()-=_+?/.,<>;:\'\"[]{}`~\\|").indexOf(keychar) > -1))
	   return true;
	else
	   return false;
}

String.prototype.toHHMMSS = function () {
    var sec_num = parseInt(this, 10); // don't forget the second param
    var hours   = Math.floor(sec_num / 3600);
    var minutes = Math.floor((sec_num - (hours * 3600)) / 60);
    var seconds = sec_num - (hours * 3600) - (minutes * 60);

    if (hours   < 10) {hours   = "0"+hours;}
    if (minutes < 10) {minutes = "0"+minutes;}
    if (seconds < 10) {seconds = "0"+seconds;}
    var time    = hours+':'+minutes+':'+seconds;
    return time;
}

$if:isJudge
function pollSubmissions()
{
	var gradeBtn = document.getElementById("gradeBtn");
	if(gradeBtn)
	{
		var xmlhttp;
		if(window.XMLHttpRequest) // code for IE7+, Firefox, Chrome, Opera, Safari
			xmlhttp = new XMLHttpRequest();
		else // code for IE6, IE5
			xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");

		xmlhttp.onreadystatechange = function()
		{
			if(xmlhttp.readyState == 4 && xmlhttp.status == 200)
			{
				var gradeBtn = document.getElementById("gradeBtn");
				gradeBtn.innerHTML = "Grade (" + xmlhttp.responseText + ")";
			}
		}
		xmlhttp.open("GET", "submissionPoll", true);
		xmlhttp.send();
	}
}
$endif:isJudge

var timeLeft = $timeLeft, totalTime = $duration;
var counter = setInterval(timer, 1000);
function timer(t)
{
	var timeText = document.getElementById("timeLeft_time");
	if(typeof(timeText) == 'undefined' || timeText == null)
	{
		clearInterval(counter);
		return;
	}
//	var percent = parseInt(timeLeft * 100 / totalTime) + "%";
	timeText.innerHTML = timeLeft.toString().toHHMMSS();
//	document.getElementById("timeLeft_bar").style.width = percent;
//	document.getElementById("timeLeft_barText").innerHTML = percent;
	timeLeft = timeLeft - 1;
	if(timeLeft < 0)
	{
		clearInterval(counter);
		return;
	}
}
setTimeout(timer, 0);

$if:isJudge
setInterval(pollSubmissions, 10000);
setTimeout(pollSubmissions, 0);
$endif:isJudge
