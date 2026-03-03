function SetMenuBar()
{
	document.body.innerHTML=`<div class="MenuBarFixed">
		<div class="MenuBar">
			<a href="/Home" class="MenuBarButton" id="PageHomeBar">
				<img class="MenuBarImg" src="img/icn/Home.svg">
				<span class="MenuBarSpan">Home</span>
			</a>
			<a href="/Dart" class="MenuBarButton" id="PageDartBar">
				<img class="MenuBarImg" src="img/icn/Dart.svg">
				<span class="MenuBarSpan">Dart</span>
			</a>
			<a href="/Score" class="MenuBarButton" id="PageScoreBar">
				<img class="MenuBarImg" src="img/icn/Score.svg">
				<span class="MenuBarSpan">Score</span>
			</a>
			<a href="/Account" class="MenuBarButton" id="PageAccountBar">
				<img class="MenuBarImg" src="img/icn/Account.svg">
				<span class="MenuBarSpan">Account</span>
			</a>
		</div>
	</div>` + document.body.innerHTML
}

SetMenuBar()
