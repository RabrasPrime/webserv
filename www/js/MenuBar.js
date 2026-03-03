async function chargerContenu() {
    try {
        const response = await fetch('http://localhost:8080/cgi/GetMenuBar.py?page=' + document.body.id);
        const html = await response.text();
        console.log("Contenu recup > " + html);
		if (response.ok)
       		document.body.insertAdjacentHTML('afterbegin', html);
		// else
		// 	console.error(`Erreur ${response.status} détectée.`);
    } catch (error) {
        console.error("Impossible de charger le contenu :", error);
    }
}

chargerContenu();