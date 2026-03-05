// {
// 	id:'PlayerId_' + id,
// 	Score:StartScore,
// 	PlayerName:player.value,
// 	ListeCoup: [],
// 	DartToPlay: 3
// }

ListPlayer = []
ListeCoup = []
NbPlayer = 0;
PlayerTourId = 0;
ScoreMultiplier = 1


function ChangeGameMode(mode)
{
	elems = document.getElementsByClassName('AllMode')
	for (elem of elems)
	{
		elem.style.display = 'none'
	}
	elems = document.getElementsByClassName(mode)
	for (elem of elems)
	{
		elem.style.display = 'flex'
	}
}

ChangeGameMode("ClassicMode")

function ChangeScore(value, name)
{
	input = document.getElementsByClassName(name)[0]
	if (value > 0 || parseInt(input.value) > -value)
		input.value = parseInt(input.value) + value
}

function RadioCheckBox(name,self)
{
	elems = document.getElementsByName(name)
	for (elem of elems)
	{
		if (elem != self)
			elem.checked = false;
	}
}

function AskPlayerName()
{
	PlayerName = prompt("Nom du Joueur :")
	// console.log(PlayerName)
	if (PlayerName != "")
	PlayerUl = document.getElementsByClassName('ListPlayerUl')[0]
	PlayerUl.innerHTML +=
`
<li>
	<button onclick="RemoveThis(this.parentElement)">x</button>
	<div>${PlayerName}</div>
	<input class="InputPlayerName" type="checkbox" value="${PlayerName}">
</li>
`
}

function PlayGame()
{
	GameMode = document.querySelector('input[name="GameModeInput"]:checked').value
	NbLeg = document.querySelector('.NbLegInput').value
	NbSet = document.querySelector('.NbSetInput').value
	StartScore = document.querySelector('.StartScoreInput').value
	CheckInType = document.querySelector('input[name="CheckInTypeInput"]:checked').value
	CheckOutType = document.querySelector('input[name="CheckOutTypeInput"]:checked').value
	Players = document.querySelectorAll('.InputPlayerName:checked')
	console.clear()
	// console.log("GameMode > " + GameMode)
	// console.log("NbLeg > " + NbLeg)
	// console.log("NbSet > " + NbSet)
	ListeCoup = []
	ListPlayer = []
	if (GameMode == 'Classic' && Players.length != 0)
	{
		
		// console.log("StartScore > " + StartScore)
		// console.log("CheckInType > " + CheckInType)
		// console.log("CheckOutType > " + CheckOutType)
		document.getElementsByClassName('PlayerListOnGame')[0].innerHTML = ''
		id = 0;
		for (player of Players)
		{
			ListPlayer.push({
								id:'PlayerId_' + id,
								Score:StartScore,
								PlayerName:player.value,
								ListeCoup: [],
								DartToPlay: 1
							})
			document.getElementsByClassName('PlayerListOnGame')[0].innerHTML +=
			`
<div class="OnePlayerDivGame" id="PlayerId_${id}">
	<div class="DivLeftPlayer">
		<div class="PlayerPlayerName">${player.value}</div>
		<div class="PlayerPlayerScore"  id="PlayerId_${id}_0">${StartScore}</div>
	</div>
	<div class="ScoreTourPlayerAll">
		<div class="ScoreTourPlayerList">
			<div id="PlayerId_${id}_1"></div>
			<div id="PlayerId_${id}_2"></div>
			<div id="PlayerId_${id}_3"></div>
		</div>
		<div class="PlayerTourScore" id="PlayerId_${id}_4">0</div>
	</div>
</div> 
			`
		id++
		NbPlayer++
		}
		document.getElementsByClassName('FixedGameAll')[0].style.display = 'flex'
		document.getElementsByClassName('GameAllDiv')[0].style.display = 'flex'
		document.getElementsByClassName('PopUpWinPlayer')[0].style.display = 'none'
	}
}

function RemoveThis(self)
{
	self.remove()
}

function AddScore(value)
{
	if (ListPlayer[PlayerTourId].DartToPlay == 1)
		document.getElementById(`PlayerId_${PlayerTourId}_4`).innerHTML = 0
	// console.log("PlayerTourId > " + PlayerTourId)
	// console.log("ListPlayer Dart > " + ListPlayer[PlayerTourId].DartToPlay)
	// console.log(value * ScoreMultiplier)
	document.getElementById(`PlayerId_${PlayerTourId}_${ListPlayer[PlayerTourId].DartToPlay}`).innerHTML = value * ScoreMultiplier
	total = parseInt(document.getElementById(`PlayerId_${PlayerTourId}_4`).innerHTML)
	total += value * ScoreMultiplier
	document.getElementById(`PlayerId_${PlayerTourId}_4`).innerHTML = total
	score = parseInt(document.getElementById(`PlayerId_${PlayerTourId}_0`).innerHTML)
	score -= value * ScoreMultiplier
	ListPlayer[PlayerTourId].ListeCoup.push(value * ScoreMultiplier)
	console.log(ListPlayer[PlayerTourId])
	if (score < 0)
	{
		for (i = 1; i <= ListPlayer[PlayerTourId].DartToPlay;i++)
		{
			remove = ListPlayer[PlayerTourId].ListeCoup.pop()
			score += remove
		}
		ListPlayer[PlayerTourId].DartToPlay = 4
	}
	else if (score == 0)
	{
		document.getElementsByClassName('GameAllDiv')[0].style.display = 'none'
		document.getElementsByClassName('PopUpWinPlayer')[0].style.display = 'flex'
		document.getElementsByClassName('TextWinnerPlayer')[0].innerHTML = `${ListPlayer[PlayerTourId].PlayerName}<br> won the game !`
		return;
	}
	document.getElementById(`PlayerId_${PlayerTourId}_0`).innerHTML = score
	// console.log(PlayerTourId)
	ListPlayer[PlayerTourId].DartToPlay++
	if (ListPlayer[PlayerTourId].DartToPlay >= 4)
	{
		ListPlayer[PlayerTourId].DartToPlay = 1
		for (i = 1;i <= 3;i++)
		{
			document.getElementById(`PlayerId_${PlayerTourId}_${i}`).innerHTML = ''
		}
		
		PlayerTourId++
	}
	if (PlayerTourId >= NbPlayer)
		PlayerTourId = 0
	ScoreMultiplier = 1
}

function ChangeMultColor()
{

}

function ChangeMultiplier(mult)
{
	if (ScoreMultiplier != mult)
		ScoreMultiplier = mult
	else
		ScoreMultiplier = 1;
	ChangeMultColor()
}

function RollBackDart()
{
	console.log(PlayerTourId)
}


function CloseGame()
{
	document.getElementsByClassName('FixedGameAll')[0].style.display='none'
	ListPlayer = []
	ListeCoup = []
	NbPlayer = 0;
	PlayerTourId = 0;
	ScoreMultiplier = 1
}