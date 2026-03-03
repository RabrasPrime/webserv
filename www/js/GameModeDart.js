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

function ChangeStartScore(value)
{
	input = document.getElementsByClassName('StartScoreInput')[0]
	if (parseInt(input.value) > 100 || value > 0)
		input.value = parseInt(input.value) + value
}