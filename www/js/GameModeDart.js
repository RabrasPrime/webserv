async function getModePage(mode)
{
	try {
        const response = await fetch('http://localhost:8080/cgi/GetModePage.py?Mode=' + mode);
        const html = await response.text();
        console.log("Contenu recup > " + html);
		if (response.ok)
			document.getElementsByClassName("GameModeParam")[0].textContent = html;
    } catch (error) {
        console.error("Impossible de charger le contenu :", error);
    }
}

function LoadMode(mode)
{
	document.getElementsByClassName("GameModeAll")[0].style.display = 'none'
	document.getElementsByClassName("GameModeParam")[0].style.display = 'flex'
	getModePage(mode)
}