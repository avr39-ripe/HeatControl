const Modes = { GAS: 1, WOOD: 2, WARMY: 4, COLDY: 8}
$( document ).ready(function() {
	
	(function worker() {
		$.getJSON('/state', function(data) {
			document.getElementById('counter').textContent = data.counter;
			document.getElementById('mode_curr_temp').innerHTML = data.mode_curr_temp + ' &deg;C';
			document.getElementById('date_time').textContent = data.date_time;
			if (data.mode & Modes.WOOD) {document.getElementById('mode').textContent = "Wood";}
			if (data.mode & Modes.WARMY) {document.getElementById('mode').textContent = "Warmy";}
			if (data.mode & Modes.COLDY) {document.getElementById('mode').textContent = "Coldy";}
			setTimeout(worker, 5000);
		});
	})();
});