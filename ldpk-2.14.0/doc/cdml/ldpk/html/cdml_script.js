// Diese Funktion wird aufgerufen, wenn man auf einen
// Indexeintrag drückt. Dann wird das referenzierte Wort
// eingefärbt.

function cdml_mark(s,k)
	{
	var i;
	var ee = [
		document.getElementsByTagName("cdml_indexitem"),
		document.getElementsByTagName("cdml_ii"),
		document.getElementsByTagName("cdml_iii")]
	for(j = 0;j < 3;j++)
		{
		e = ee[j]
// Alle Hintergruende auf normal setzen
		for(i = 0; i < e.length;i++)
			{ e[i].style.backgroundColor = "inherit"; }
// ALle passenden Elemente markieren
		for(i = 0; i < e.length;i++)
			{
			if((e[i].getAttribute('keyword') == k) || (e[i].innerText == k))
				{ e[i].style.backgroundColor = "#cf7"; }
			}
		}
	window.location.hash = s;
	}

