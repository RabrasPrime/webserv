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
	console.log(PlayerName)
	if (PlayerName != "")
	PlayerUl = document.getElementsByClassName('ListPlayerUl')[0]
	PlayerUl.innerHTML +=
`
<li>
	<div>${PlayerName}</div>
	<input type="checkbox" value="${PlayerName}">
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
	console.clear()
	console.log("GameMode > " + GameMode)
	console.log("NbLeg > " + NbLeg)
	console.log("NbSet > " + NbSet)
	if (GameMode == 'Classic')
	{
		console.log("StartScore > " + StartScore)
		console.log("CheckInType > " + CheckInType)
		console.log("CheckOutType > " + CheckOutType)
	}
}

function RemoveThis(self)
{
}