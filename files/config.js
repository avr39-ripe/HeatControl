function get_config() {
	$.getJSON('/config.json',
			function(data) {
				$.each(data, function(key, value){
            		document.getElementById(key).value = value;
        		});
            });
}

function post_config(event) {
	event.preventDefault();
	var formData = {
			'mode_switch_temp'			:	document.getElementById('mode_switch_temp').value,
			'mode_switch_temp_delta'	:	document.getElementById('mode_switch_temp_delta').value,
			'pump_on_delay'				:	document.getElementById('pump_on_delay').value,
			'pump_off_delay'			:	document.getElementById('pump_off_delay').value,
			'caldron_on_delay'			:	document.getElementById('caldron_on_delay').value
			};
	$.ajax({
        type        : 'POST',
        url         : '/config',
        data        : formData
    })
}

$( document ).ready(function() {
	get_config();
	
	document.getElementById('form_settings').addEventListener('submit', post_config);
});