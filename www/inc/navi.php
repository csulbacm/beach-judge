

<nav class="navbar navbar-default" role="navigation">
	<div class="container">
		<div class="row">
			<div class="col-md-12">
				
	<ul class="nav navbar-nav">
		<li>
			<a href="#">
				<strong>
					beachjudge
				</strong>
			</a>
		</li>
	</ul>
			
	<ul class="nav navbar-nav pull-right">
		$if:loggedIn
			<li><a href="#">
			Leaderboard
			</a></li>
			<li><a href="#">
			Submit
			</a></li>
			<li><a href="#">
			Profile
			</a></li>
		$endif:loggedIn
		$if:loggedOut
			<li>
				<a href="login.html" class="btn btn-default" style="padding:10px; margin:5px 0;">
					Login
				</a>
			</li>
		$endif:loggedOut
		
	</ul>
			</div>
		</div>
	</div>
</nav>