if (window.location.protocol != 'https:')
	window.location.href = 'https:' + window.location.href.substring(window.location.protocol.length);

function setCookie(cname, cvalue, exdays) {
	var d = new Date();
	d.setTime(d.getTime() + (exdays * 24 * 60 * 60 * 1000));
	var expires = 'expires=' + d.toUTCString();
	document.cookie = cname + '=' + cvalue + ';' + expires;
}

$(document).ready(function() {
	$("#login-form").submit(function(evt) {
		$('.login-error').hide();
		$.ajax({
			type: 'POST',
			url: '/',
			data: $(this).serialize(),
			success: function(result) {
				if(result === 'ERR')
					$('.login-error').show();
				else {
					sessionStorage.setItem('JUDGESESSID', result);
					setCookie('JUDGESESSID', result, 1);
					window.location = window.location;
				}
			}
		});
		return false;
	});
});
