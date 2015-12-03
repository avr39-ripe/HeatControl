function get_config() {
	$.getJSON('/config.json',
			function(data) {
				$.each(data, function(key, value){
            		document.getElementById(key).value = value;
            	if (data.sta_enable == 1) {
            		document.getElementById('sta_enable').checked = true;
            	}
            	else
            		document.getElementById('sta_enable').checked = false;
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
			'caldron_on_delay'			:	document.getElementById('caldron_on_delay').value,
			'room_off_delay'			:	document.getElementById('room_off_delay').value,
			'start_minutes'				:	document.getElementById('start_minutes').value,
			'stop_minutes'				:	document.getElementById('stop_minutes').value,
			'cycle_duration'			:	document.getElementById('cycle_duration').value,
			'cycle_interval'			:	document.getElementById('cycle_interval').value
			};
	$.ajax({
        type        : 'POST',
        url         : '/config',
        data        : formData
    })
}

function post_hwpump(event) {
	event.preventDefault();
	var formData = {
			'start_minutes'				:	document.getElementById('start_minutes').value,
			'stop_minutes'				:	document.getElementById('stop_minutes').value,
			'cycle_duration'			:	document.getElementById('cycle_duration').value,
			'cycle_interval'			:	document.getElementById('cycle_interval').value
			};
	$.ajax({
        type        : 'POST',
        url         : '/config',
        data        : formData
    })
}

function post_netcfg(event) {
	event.preventDefault();
	var formData = {
			'SSID'					:	document.getElementById('SSID').value,
			'Password'				:	document.getElementById('Password').value,
			'sta_enable'			:	(document.getElementById('sta_enable').checked ? 1 : 0)
			};
	$.ajax({
        type        : 'POST',
        url         : '/config',
        data        : formData
    })
}

function post_datetime(event) {
	event.preventDefault();
	d = new Date()
	var formData = {
			'time_zone'				:	Math.abs(d.getTimezoneOffset()/60),
			'Second'				:	d.getUTCSeconds(),
			'Minute'				:	d.getUTCMinutes(),
			'Hour'					:	d.getUTCHours(),
			'Wday'					:	d.getUTCDay(),
			'Day'					:	d.getUTCDate(),
			'Month'					:	d.getUTCMonth(),
			'Year'					:	d.getUTCFullYear()
			};
	$.ajax({
        type        : 'POST',
        url         : '/datetime',
        data        : formData
    })
}

$( document ).ready(function() {
	get_config();
	
	document.getElementById('form_netcfg').addEventListener('submit', post_netcfg);
	document.getElementById('netcfg_cancel').addEventListener('click', get_config);
	document.getElementById('form_settings').addEventListener('submit', post_config);
	document.getElementById('settings_cancel').addEventListener('click', get_config);
	document.getElementById('form_hwpump').addEventListener('submit', post_hwpump);
	document.getElementById('hwpump_cancel').addEventListener('click', get_config);
	document.getElementById('sync_datetime').addEventListener('click', post_datetime);
});